
#include <string>
#include <map>
#include "DocumentIndexImpl.h"
#include <memory>
#include <utility>
#include "QueryParser.hpp"
#include <set>

using namespace std;

class Engine
{
	public:

		Engine(const string& queryParserDelimiters, char keyWordSplitter) :
			queryParserDelimiters(queryParserDelimiters),
			keyWordSplitter(keyWordSplitter)
		{
			documentIndex = new DocumentIndexImpl();
		}

		~Engine()
		{
			delete documentIndex;
		}

		void addDocument(shared_ptr<IDocument> document)
		{
			documentIndex->addDoc(docId, document);

			auto entries = document->getEntries();

			for (auto iter = entries.begin(); iter != entries.end(); ++iter)
			{
				string key = iter->first;
				string query = iter->second;

				QueryParser qp(query, queryParserDelimiters);

				for (auto token : qp.getTokens())
				{
					string word = key + keyWordSplitter + token;

					auto found = wordIndex.find(word);

					/*
					 * if the word exists in the wordIndex then
					 * we know that a reverse index for it exists
					 */
					if (found != wordIndex.end())
					{
						auto wordId = found->second;
						invertedIndex[wordId].insert(docId);
					}
					else // this is a brand new word
					{
						set<unsigned int> docSet;
						docSet.insert(docId);

						invertedIndex.insert(make_pair(wordId, docSet));
						wordIndex.insert(make_pair(word, wordId++));
					}
				}

			} // end looping through entries

			++docId;
		}

		set<shared_ptr<IDocument>> getDocumentList(const string& word)
		{
			set<shared_ptr<IDocument>> documentSet;

			auto found = wordIndex.find(word);

			if (found != wordIndex.end())
			{
				auto documents = documentIndex->getDocuments();

				auto wordId = found->second;
				auto docSet = invertedIndex[wordId];

				for (auto id : docSet)
				{
					documentSet.insert(documents[id]);
				}
			}

			return move(documentSet);
		}

		set<string> getWords()
		{
			set<string> words;

			for (auto iter = wordIndex.begin(); iter != wordIndex.end(); ++iter)
			{
				words.insert(iter->first);
			}

			return move(words);
		}

	private:

		unsigned long docId = 1;
		unsigned long wordId = 1;

		string queryParserDelimiters;
		char keyWordSplitter;

		// store all the words
		map<string, unsigned int> wordIndex;

		// store all the documents
		IDocumentIndex* documentIndex;

		// inverted index that maps words(wordId) to documents that contain it
		map<unsigned int, set<unsigned int>> invertedIndex;

};
