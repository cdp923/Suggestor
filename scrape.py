import requests
from bs4 import BeautifulSoup
import sqlite3
import time
import urllib.parse
from datetime import datetime
import sys
import re
from urllib.robotparser import RobotFileParser
from collections import deque
import asyncio
from playwright.async_api import async_playwright, Playwright, Browser, BrowserContext

# --- Configuration ---
DATABASE_NAME = 'autocorrect_words.db'
USER_AGENT = 'MyPythonAutocorrectScraper/1.0 (webscrapetest11@gmail.com)' # IMPORTANT: Use a real contact!
START_URLS = [
    # "https://www.gutenberg.org/files/1342/1342-0.txt",
    # "https://www.gutenberg.org/browse/scores/top",
    "https://www.merriam-webster.com/browse/dictionary/a", # Fixed: Added specific letter
    # Add other URLs you have permission to scrape.
]
MAX_PAGES_TO_SCRAPE = 500000
CRAWL_DELAY_DEFAULT = 1.1 # Increased delay for better politeness
MAX_CONCURRENT_PAGES = 2 # Reduced concurrency to be more respectful

# --- Database Schema (MUST MATCH C++ DEFINITION) ---
CREATE_WORDS_TABLE_SQL = """
CREATE TABLE IF NOT EXISTS words (
    word_text TEXT PRIMARY KEY NOT NULL,
    frequency INTEGER DEFAULT 0,
    last_used_timestamp TEXT,
    source TEXT NOT NULL
);
"""

CREATE_TYPOS_TABLE_SQL = """
CREATE TABLE IF NOT EXISTS common_typos (
    typo_text TEXT PRIMARY KEY NOT NULL,
    correct_word_text TEXT NOT NULL,
    confidence INTEGER DEFAULT 1,
    FOREIGN KEY (correct_word_text) REFERENCES words(word_text)
);
"""

# --- Database Manager Functions ---
def get_db_connection():
    """Establishes and returns a connection to the SQLite database."""
    try:
        conn = sqlite3.connect(DATABASE_NAME)
        conn.row_factory = sqlite3.Row # Allows accessing columns by name
        return conn
    except sqlite3.Error as e:
        print(f"Database connection error: {e}", file=sys.stderr)
        sys.exit(1)

def initialize_db_schema(conn):
    """Initializes the database schema (creates tables if they don't exist)."""
    cursor = conn.cursor()
    try:
        cursor.execute(CREATE_WORDS_TABLE_SQL)
        cursor.execute(CREATE_TYPOS_TABLE_SQL)
        conn.commit()
        print("Database schema initialized successfully.")
        return True
    except sqlite3.Error as e:
        print(f"Error initializing schema: {e}", file=sys.stderr)
        return False

# Modified for batch upsert
def bulk_upsert_word_frequency(conn, word_counts, source):
    """
    Inserts or updates word frequencies in bulk.
    """
    if not word_counts:  # Check if dictionary is empty
        return True
        
    cursor = conn.cursor()
    now_timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    sql_upsert = """
    INSERT INTO words (word_text, frequency, last_used_timestamp, source)
    VALUES (?, ?, ?, ?)
    ON CONFLICT(word_text) DO UPDATE SET
        frequency = frequency + excluded.frequency,
        last_used_timestamp = ?;
    """
    data_to_upsert = []
    for word, count in word_counts.items():
        # Filter out very short or long words that are likely not useful
        if 2 <= len(word) <= 50:
            data_to_upsert.append((word, count, now_timestamp, source, now_timestamp))

    if not data_to_upsert:
        return True

    try:
        cursor.executemany(sql_upsert, data_to_upsert)
        conn.commit()
        return True
    except sqlite3.Error as e:
        print(f"Error bulk upserting words: {e}", file=sys.stderr)
        return False

def add_common_typo(conn, typo_text, correct_word_text, confidence=1):
    """Adds or updates a common typo mapping."""
    cursor = conn.cursor()
    sql = """
    INSERT OR REPLACE INTO common_typos (typo_text, correct_word_text, confidence)
    VALUES (?, ?, ?);
    """
    try:
        cursor.execute(sql, (typo_text, correct_word_text, confidence))
        conn.commit()
        print(f"Added typo '{typo_text}' -> '{correct_word_text}'.")
        return True
    except sqlite3.Error as e:
        print(f"Error adding typo '{typo_text}': {e}", file=sys.stderr)
        return False

# --- Web Scraping Helper Functions ---
def get_domain_from_url(url):
    """Extracts the base domain (scheme + netloc) from a given URL."""
    parsed_url = urllib.parse.urlparse(url)
    return f"{parsed_url.scheme}://{parsed_url.netloc}"

# Cache for parsed robots.txt rules to avoid re-fetching for the same domain
robots_parsers = {}
domain_crawl_delays = {} # Cache for computed crawl delays per domain

# Dictionary to track last access time per domain for rate limiting
domain_last_access_times = {} 
domain_access_locks = {} # For asynchronous rate limiting

async def fetch_robots_txt_rules_async(domain_url, user_agent):
    """
    Asynchronously fetches and parses the robots.txt file for a given domain.
    Utilizes caching to only download it once per domain.
    Also caches the crawl delay for the user agent.
    """
    if domain_url in robots_parsers:
        return robots_parsers[domain_url]

    robots_url = urllib.parse.urljoin(domain_url, '/robots.txt')
    rp = RobotFileParser()
    rp.set_url(robots_url)
    try:
        # Use asyncio with requests for robots.txt
        loop = asyncio.get_event_loop()
        headers = {'User-Agent': user_agent}
        
        # Run the blocking request in a thread pool
        response = await loop.run_in_executor(
            None, 
            lambda: requests.get(robots_url, headers=headers, timeout=10)
        )
        response.raise_for_status()
        rp.parse(response.text.splitlines())
        robots_parsers[domain_url] = rp
        delay = rp.crawl_delay(user_agent)
        domain_crawl_delays[domain_url] = delay if delay is not None else CRAWL_DELAY_DEFAULT
        print(f"Fetched and parsed robots.txt for {domain_url}. Crawl delay: {domain_crawl_delays[domain_url]}s")
        return rp
    except requests.exceptions.RequestException as e:
        print(f"Error fetching robots.txt for {domain_url}: {e}", file=sys.stderr)
        # Default to allow if robots.txt cannot be fetched, but warn
        domain_crawl_delays[domain_url] = CRAWL_DELAY_DEFAULT
        robots_parsers[domain_url] = None  # Cache the failure
        return None
    except Exception as e:
        print(f"Error parsing robots.txt for {domain_url}: {e}", file=sys.stderr)
        domain_crawl_delays[domain_url] = CRAWL_DELAY_DEFAULT
        robots_parsers[domain_url] = None
        return None

def is_url_allowed(url, user_agent):
    """Checks if a URL is allowed by robots.txt for the given user agent."""
    domain_url = get_domain_from_url(url)
    rp = robots_parsers.get(domain_url)
    if rp:
        return rp.can_fetch(user_agent, url)
    return True # Default to allow if no robots.txt rules could be fetched

# --- Content Fetching (Playwright Async) ---
async def playwright_fetch_url_content_async(url: str, user_agent: str, context: BrowserContext):
    """
    Fetches content from a URL using a headless browser (Playwright async).
    Reuses an existing browser context.
    """
    print(f"Fetching (Playwright): {url}")
    page = None
    try:
        page = await context.new_page()

        # Block resource types for faster loading
        await page.route(re.compile(r"\.(png|jpg|jpeg|gif|css|woff|woff2|ttf|svg|mp4|webm|webp|ico)"),
                         lambda route: route.abort())
        await page.route(re.compile(r"google-analytics\.com|googletagmanager\.com|doubleclick\.net|adservice\.google\.com|facebook\.com|twitter\.com"),
                         lambda route: route.abort())

        await page.goto(url, wait_until="domcontentloaded", timeout=30000)
        content = await page.content()
        return content
    except Exception as e:
        print(f"Error fetching with Playwright from {url}: {e}", file=sys.stderr)
        return None
    finally:
        if page:
            await page.close()

def extract_words_from_text(text_content):
    """
    Extracts words from text, converting to lowercase and removing punctuation.
    Uses regex for more robust word tokenization.
    """
    if not text_content:
        return []
    
    # More refined regex to handle contractions and hyphenated words better
    words = re.findall(r"[a-zA-Z]+(?:[''-][a-zA-Z]+)*", text_content.lower())
    
    # Filter out very short words and clean up
    filtered_words = []
    for word in words:
        # Remove leading/trailing punctuation and check length
        clean_word = re.sub(r"^['''-]+|['''-]+$", "", word)
        if 2 <= len(clean_word) <= 50 and clean_word.isalpha():
            filtered_words.append(clean_word)
    
    return filtered_words

def get_all_links_from_html(html_content, base_url):
    """
    Extracts all absolute links from HTML content.
    Filters to keep only HTTP/HTTPS links and stays within the target dictionary domain.
    """
    if not html_content:
        return []
        
    soup = BeautifulSoup(html_content, 'html.parser')
    links = set()
    
    # Define acceptable URL prefixes within Merriam-Webster
    acceptable_prefixes = [
        "https://www.merriam-webster.com/browse/dictionary/",
        "https://www.merriam-webster.com/dictionary/"
    ]

    for a_tag in soup.find_all('a', href=True):
        href = a_tag['href']
        try:
            absolute_url = urllib.parse.urljoin(base_url, href)
            
            # Only add HTTP/HTTPS links
            if absolute_url.startswith(('http://', 'https://')):
                # Check if the URL starts with any of the acceptable prefixes
                for prefix in acceptable_prefixes:
                    if absolute_url.startswith(prefix):
                        # Clean up fragment identifiers and query parameters
                        parsed_url = urllib.parse.urlparse(absolute_url)
                        clean_url = urllib.parse.urlunparse(
                            parsed_url._replace(fragment='', query='')
                        )
                        links.add(clean_url)
                        break
        except Exception as e:
            print(f"Error processing link {href}: {e}")
            continue
    
    return list(links)

# --- Main Asynchronous Scraping Logic ---

# Global counters and queues
url_queue: asyncio.Queue = None
visited_urls: set = set()
pages_scraped_count: int = 0

async def acquire_domain_lock(domain_url: str):
    """Acquires a lock for a domain, respecting crawl delays."""
    if domain_url not in domain_access_locks:
        domain_access_locks[domain_url] = asyncio.Lock()
    
    lock = domain_access_locks[domain_url]
    await lock.acquire()
    
    try:
        current_delay = domain_crawl_delays.get(domain_url, CRAWL_DELAY_DEFAULT)
        last_access = domain_last_access_times.get(domain_url, 0)
        time_since_last_access = time.time() - last_access

        if time_since_last_access < current_delay:
            wait_time = current_delay - time_since_last_access
            print(f"[{domain_url}] Waiting for {wait_time:.2f}s before next request.")
            await asyncio.sleep(wait_time)
        
        domain_last_access_times[domain_url] = time.time()
        return lock  # Return the lock so it can be released later
    except Exception as e:
        lock.release()  # Release on error
        raise e

async def handle_url_task(conn: sqlite3.Connection, url: str, user_agent: str, context: BrowserContext):
    global pages_scraped_count

    domain_url = get_domain_from_url(url)
    lock = None
    
    try:
        # Ensure robots.txt is fetched and delay information is available
        if domain_url not in robots_parsers:
            await fetch_robots_txt_rules_async(domain_url, user_agent)

        # Check robots.txt permission
        if not is_url_allowed(url, user_agent):
            print(f"Skipping {url}: Disallowed by robots.txt.")
            return

        # Acquire domain lock and apply delay
        lock = await acquire_domain_lock(domain_url)

        print(f"Processing URL ({pages_scraped_count + 1}/{MAX_PAGES_TO_SCRAPE}): {url}")
        
        # Increment the counter
        pages_scraped_count += 1

        content = await playwright_fetch_url_content_async(url, user_agent, context)
        if content is None:
            print(f"Failed to fetch content from {url}.")
            return

        plain_text = ""
        new_links = []

        if url.endswith(".txt"):
            plain_text = content
        else:
            soup = BeautifulSoup(content, 'html.parser')
            # More aggressive cleanup of non-textual elements
            for element in soup(['script', 'style', 'header', 'footer', 'nav', 'aside', 
                               'img', 'svg', 'form', 'noscript', 'iframe', 'embed', 'object']):
                element.extract()
            plain_text = soup.get_text(separator=' ', strip=True)

            # Extract new links for crawling
            new_links = get_all_links_from_html(content, url)
            added_count = 0
            for link in new_links:
                if (link not in visited_urls and 
                    pages_scraped_count + url_queue.qsize() < MAX_PAGES_TO_SCRAPE):
                    visited_urls.add(link)
                    await url_queue.put(link)
                    added_count += 1
                    if added_count >= 10:  # Limit new links per page
                        break

        words = extract_words_from_text(plain_text)
        word_counts = {}
        for word in words:
            #word_counts[word] = word_counts.get(word, 0) + 1
            word_counts[word] = 1

        print(f"Extracted {len(words)} raw words, {len(word_counts)} unique words from {url}.")

        # Bulk upsert into database
        if word_counts:  # Only update if we have words
            if not bulk_upsert_word_frequency(conn, word_counts, 'web_scrape'):
                print(f"Failed to bulk upsert words for {url}.")

    except Exception as e:
        print(f"An unexpected error occurred while processing {url}: {e}", file=sys.stderr)
    finally:
        # Ensure the lock is released
        if lock and lock.locked():
            lock.release()

async def main():
    global url_queue
    global pages_scraped_count

    conn = get_db_connection()
    if not initialize_db_schema(conn):
        conn.close()
        sys.exit(1)

    url_queue = asyncio.Queue()
    for start_url in START_URLS:
        await url_queue.put(start_url)
        visited_urls.add(start_url)

    print(f"\nStarting asynchronous web scraping process with max concurrency of {MAX_CONCURRENT_PAGES}...")

    # Launch Playwright browser and context once
    async with async_playwright() as p:
        browser = await p.chromium.launch(headless=True)
        context = await browser.new_context(
            accept_downloads=False, 
            user_agent=USER_AGENT,
            java_script_enabled=True,
            viewport={'width': 1280, 'height': 720}
        )

        processing_tasks = set()

        try:
            while pages_scraped_count < MAX_PAGES_TO_SCRAPE:
                # Add new tasks if queue has URLs and we are below max concurrency
                while (not url_queue.empty() and 
                       len(processing_tasks) < MAX_CONCURRENT_PAGES and
                       pages_scraped_count < MAX_PAGES_TO_SCRAPE):
                    
                    try:
                        current_url = await asyncio.wait_for(url_queue.get(), timeout=1.0)
                        
                        task = asyncio.create_task(
                            handle_url_task(conn, current_url, USER_AGENT, context)
                        )
                        processing_tasks.add(task)
                        task.add_done_callback(processing_tasks.discard)
                        
                        url_queue.task_done()
                    except asyncio.TimeoutError:
                        break

                # If no tasks are running and no URLs are in the queue, we're done
                if not processing_tasks and url_queue.empty():
                    print("No more URLs to process and no active tasks. Exiting.")
                    break

                # Wait for at least one task to complete
                if processing_tasks:
                    done, pending = await asyncio.wait(
                        processing_tasks, 
                        return_when=asyncio.FIRST_COMPLETED,
                        timeout=5.0
                    )
                    
                    # Check for exceptions in completed tasks
                    for task in done:
                        try:
                            await task  # This will raise any exception that occurred
                        except Exception as e:
                            print(f"Task failed with error: {e}")

                else:
                    # No tasks running, short sleep to prevent busy waiting
                    await asyncio.sleep(0.1)

        except KeyboardInterrupt:
            print("\nReceived interrupt signal. Stopping gracefully...")
        finally:
            # Wait for any remaining tasks to complete gracefully
            if processing_tasks:
                print(f"Waiting for {len(processing_tasks)} remaining tasks to complete...")
                await asyncio.gather(*processing_tasks, return_exceptions=True)

            await context.close()
            await browser.close()

    conn.close()
    print(f"\nFinished scraping {pages_scraped_count} pages (max {MAX_PAGES_TO_SCRAPE}).")
    print("Scraping and database population complete.")

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nScraping interrupted by user (Ctrl+C). Exiting gracefully.")
    except Exception as e:
        print(f"A critical error occurred: {e}", file=sys.stderr)
        sys.exit(1)