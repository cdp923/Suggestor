import csv
from wordfreq import word_frequency, iter_wordlist

def generate_wordfreq_csv(output_file, lang='en'):
    """
    Generates a CSV file containing common words and their frequencies.
    The frequency is a score from 0 to 1, where 1 is the most frequent.
    """
    print(f"Generating word frequency data for language '{lang}'...")
    
    
    with open(output_file, 'w', newline='', encoding='utf-8') as csvfile:
        fieldnames = ['word', 'frequency']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        
        # Write the header row
        writer.writeheader()
        
        # Iterate over the word list and write each word and its frequency to the file
        for word in iter_wordlist(lang, 'large'): # 'large' gets the top 100,000 words
            freq = word_frequency(word, lang)
            writer.writerow({'word': word, 'frequency': freq})
            
    print(f"Successfully generated {output_file}.")

if __name__ == "__main__":
    # Define the output file name
    output_filename = "wordfreq_data.csv"
    # Run the function to create the file
    generate_wordfreq_csv(output_filename)