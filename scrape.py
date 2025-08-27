import requests
from bs4 import BeautifulSoup
import sqlite3
import time
import urllib.parse
from datetime import datetime
import sys
import re
from urllib.robotparser import RobotFileParser
import asyncio
from playwright.async_api import async_playwright
from collections import Counter

# --- Configuration ---
DATABASE_NAME = 'data/my_dictionary.db'  # Path to your existing database
USER_AGENT = 'AutocorrectFrequencyUpdater/1.0 (your-email@example.com)'
START_URLS = [
    "https://www.gutenberg.org/files/1342/1342-0.txt",  # Pride and Prejudice
    "https://www.gutenberg.org/files/11/11-0.txt",      # Alice in Wonderland
    "https://www.gutenberg.org/files/84/84-0.txt",      # Frankenstein
    "https://www.gutenberg.org/files/1661/1661-0.txt",  # Sherlock Holmes
    # Add more URLs as needed
]
MAX_PAGES_TO_SCRAPE = 50
CRAWL_DELAY_DEFAULT = 2.0
BATCH_SIZE = 1000  # Process words in batches for better performance

# --- Database Functions ---
def get_db_connection():
    """Establishes connection to your existing SQLite database."""
    try:
        conn = sqlite3.connect(DATABASE_NAME)
        conn.row_factory = sqlite3.Row
        return conn
    except sqlite3.Error as e:
        print(f"Database connection error: {e}", file=sys.stderr)
        sys.exit(1)

def check_database_schema(conn):
    """Verifies that the dictionary table exists with expected structure."""
    cursor = conn.cursor()
    try:
        cursor.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='dictionary';")
        if not cursor.fetchone():
            print("Error: 'dictionary' table not found in database!", file=sys.stderr)
            return False
        
        # Check if table has expected columns
        cursor.execute("PRAGMA table_info(dictionary);")
        columns = [row[1] for row in cursor.fetchall()]
        expected_columns = ['word', 'frequency', 'partOfSpeech', 'time', 'source']
        
        missing_columns = [col for col in expected_columns if col not in columns]
        if missing_columns:
            print(f"Warning: Missing expected columns: {missing_columns}")
        
        print(f"Database schema verified. Columns found: {columns}")
        return True
        
    except sqlite3.Error as e:
        print(f"Error checking database schema: {e}", file=sys.stderr)
        return False

def get_existing_words(conn):
    """Retrieves all words from the dictionary table for comparison."""
    cursor = conn.cursor()
    try:
        cursor.execute("SELECT word FROM dictionary;")
        existing_words = set(row[0].lower() for row in cursor.fetchall())
        print(f"Loaded {len(existing_words)} existing words from database.")
        return existing_words
    except sqlite3.Error as e:
        print(f"Error retrieving existing words: {e}", file=sys.stderr)
        return set()

def update_word_frequencies(conn, word_frequency_updates):
    """Updates frequencies for words that exist in the database."""
    if not word_frequency_updates:
        return True
    
    cursor = conn.cursor()
    current_time = int(time.time())
    
    update_sql = """
    UPDATE dictionary 
    SET frequency = frequency + ?, time = ? 
    WHERE word = ?;
    """
    
    updates_data = []
    for word, freq_increment in word_frequency_updates.items():
        updates_data.append((freq_increment, current_time, word))
    
    try:
        if len(updates_data) <= 5:
            examples = updates_data
        else:
            examples = updates_data[:5]
        
        print(f"Updating {len(updates_data)} words. Examples:")
        for freq_inc, time_val, word in examples:
            print(f"  '{word}': +{freq_inc} frequency")
        if len(updates_data) > 5:
            print(f"  ... and {len(updates_data) - 5} more")
        
        cursor.executemany(update_sql, updates_data)
        rows_affected = cursor.rowcount
        conn.commit()
        print(f"Successfully updated frequencies for {rows_affected} words.")
        return True
    except sqlite3.Error as e:
        print(f"Error updating word frequencies: {e}", file=sys.stderr)
        conn.rollback()
        return False

def get_word_stats(conn):
    """Returns basic statistics about the dictionary."""
    cursor = conn.cursor()
    try:
        cursor.execute("SELECT COUNT(*), AVG(frequency), MAX(frequency) FROM dictionary;")
        count, avg_freq, max_freq = cursor.fetchone()
        
        cursor.execute("SELECT COUNT(*) FROM dictionary WHERE partOfSpeech IS NULL;")
        no_pos_count = cursor.fetchone()[0]
        
        return {
            'total_words': count,
            'avg_frequency': round(avg_freq, 2) if avg_freq else 0,
            'max_frequency': max_freq or 0,
            'words_without_pos': no_pos_count
        }
    except sqlite3.Error as e:
        print(f"Error getting word statistics: {e}", file=sys.stderr)
        return None

# --- Text Processing Functions ---
def extract_and_clean_words(text_content):
    """Extracts words from text, supporting both uppercase and lowercase letters."""
    if not text_content:
        return []
    
    words = re.findall(r'[a-zA-Z]+', text_content)
    
    clean_words = []
    for word in words:
        word_lower = word.lower()
        if all((65 <= ord(char) <= 90) or (97 <= ord(char) <= 122) for char in word):
            if 2 <= len(word_lower) <= 50:
                clean_words.append(word_lower)
    
    return clean_words

def count_word_frequencies(words):
    """Counts frequency of each word in the list."""
    return Counter(words)

# --- Web Scraping Functions ---
robots_parsers = {}

def get_domain_from_url(url):
    """Extracts domain from URL."""
    parsed_url = urllib.parse.urlparse(url)
    return f"{parsed_url.scheme}://{parsed_url.netloc}"

def fetch_robots_txt(domain_url, user_agent):
    """Fetches and parses robots.txt for a domain."""
    if domain_url in robots_parsers:
        return robots_parsers[domain_url]
    
    robots_url = urllib.parse.urljoin(domain_url, '/robots.txt')
    rp = RobotFileParser()
    rp.set_url(robots_url)
    
    try:
        headers = {'User-Agent': user_agent}
        response = requests.get(robots_url, headers=headers, timeout=10)
        response.raise_for_status()
        rp.parse(response.text.splitlines())
        robots_parsers[domain_url] = rp
        return rp
    except requests.exceptions.RequestException as e:
        print(f"Could not fetch robots.txt for {domain_url}: {e}")
        robots_parsers[domain_url] = None
        return None

def is_url_allowed(url, user_agent):
    """Checks if URL is allowed by robots.txt."""
    domain_url = get_domain_from_url(url)
    rp = robots_parsers.get(domain_url)
    if rp:
        return rp.can_fetch(user_agent, url)
    return True

async def fetch_url_content(url, user_agent, context):
    """Fetches content from URL using Playwright."""
    print(f"Fetching: {url}")
    page = None
    try:
        page = await context.new_page()
        
        # Block unnecessary resources for faster loading
        await page.route(re.compile(r"\.(png|jpg|jpeg|gif|css|woff|woff2|ttf|svg|mp4|webm|webp|ico)"),
                         lambda route: route.abort())
        
        await page.goto(url, wait_until="domcontentloaded", timeout=30000)
        content = await page.content()
        return content
    except Exception as e:
        print(f"Error fetching {url}: {e}", file=sys.stderr)
        return None
    finally:
        if page:
            await page.close()

def extract_text_from_html(html_content):
    """Extracts plain text from HTML content."""
    if not html_content:
        return ""
    
    soup = BeautifulSoup(html_content, 'html.parser')
    
    # Remove script, style, and other non-content elements
    for element in soup(['script', 'style', 'header', 'footer', 'nav', 'aside', 
                        'img', 'svg', 'form', 'noscript', 'iframe', 'embed', 'object']):
        element.extract()
    
    return soup.get_text(separator=' ', strip=True)

# --- Main Processing Logic ---
async def process_url(conn, url, user_agent, context, existing_words):
    """Processes a single URL and updates word frequencies."""
    domain_url = get_domain_from_url(url)
    
    # Check robots.txt
    if domain_url not in robots_parsers:
        fetch_robots_txt(domain_url, user_agent)
    
    if not is_url_allowed(url, user_agent):
        print(f"Skipping {url}: Disallowed by robots.txt")
        return 0
    
    # Fetch content
    content = await fetch_url_content(url, user_agent, context)
    if not content:
        return 0
    
    # Extract text
    if url.endswith('.txt'):
        text_content = content
    else:
        text_content = extract_text_from_html(content)
    
    # Extract and count words
    words = extract_and_clean_words(text_content)
    word_counts = count_word_frequencies(words)
    
    # Filter to only words that exist in database
    existing_word_counts = {
        word: count for word, count in word_counts.items() 
        if word in existing_words
    }
    
    print(f"Found {len(words)} total words, {len(existing_word_counts)} exist in database")
    
    # Update database
    if existing_word_counts:
        if update_word_frequencies(conn, existing_word_counts):
            return len(existing_word_counts)
    
    return 0

async def main():
    """Main scraping function."""
    print("Starting word frequency updater...")
    
    conn = get_db_connection()
    if not check_database_schema(conn):
        conn.close()
        sys.exit(1)
    
    # Get initial stats
    initial_stats = get_word_stats(conn)
    if initial_stats:
        print(f"Initial database stats: {initial_stats}")
    
    # Load existing words
    existing_words = get_existing_words(conn)
    if not existing_words:
        print("No existing words found in database!")
        conn.close()
        sys.exit(1)
    
    total_updates = 0
    urls_processed = 0
    
    # Launch Playwright
    async with async_playwright() as p:
        browser = await p.chromium.launch(headless=True)
        context = await browser.new_context(
            user_agent=USER_AGENT,
            viewport={'width': 1280, 'height': 720}
        )
        
        try:
            for url in START_URLS[:MAX_PAGES_TO_SCRAPE]:
                print(f"\nProcessing URL {urls_processed + 1}/{min(len(START_URLS), MAX_PAGES_TO_SCRAPE)}: {url}")
                
                updates = await process_url(conn, url, USER_AGENT, context, existing_words)
                total_updates += updates
                urls_processed += 1
                
                # Respect crawl delay
                await asyncio.sleep(CRAWL_DELAY_DEFAULT)
                
        except KeyboardInterrupt:
            print("\nInterrupted by user. Stopping gracefully...")
        finally:
            await context.close()
            await browser.close()
    
    # Final stats
    final_stats = get_word_stats(conn)
    conn.close()
    
    print(f"\nScraping Complete")
    print(f"URLs processed: {urls_processed}")
    print(f"Total word frequency updates: {total_updates}")
    
    if initial_stats and final_stats:
        print(f"Average frequency changed from {initial_stats['avg_frequency']} to {final_stats['avg_frequency']}")
        print(f"Max frequency changed from {initial_stats['max_frequency']} to {final_stats['max_frequency']}")

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nScraping interrupted by user.")
    except Exception as e:
        print(f"Critical error: {e}", file=sys.stderr)
        sys.exit(1)