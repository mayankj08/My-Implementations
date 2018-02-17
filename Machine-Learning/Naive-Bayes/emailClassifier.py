import sys
import os
from math import log

class emailClassifier:
    def __init__(self,arg):
        self.trainingHamDirectory= arg[1]
        self.trainingSpamDirectory= arg[2]
        self.testingHamDirectory = arg[3]
        self.testingSpamDirectory = arg[4]
        self.ignoreStopWords = arg[5]

        self.ham = dict()
        self.spam = dict ()
        self.stopWords = []
        self.dictionary = dict()
        self.ignoreWords = []

        self.totalDocuments = 0
        self.totalSpamDocuments = 0

        self.probabilityOfSpamDocument = 0
        self.probabilityOfHamDocument = 0

        self.stopWordsFile = "stopWords.txt"
        self.ignoreWordsFile = "ignoreWords.txt"

        self.ignoreSubjectLine = False

    def readIgnoreWords(self):
        with open(self.ignoreWordsFile) as f:
            [self.ignoreWords.append(x.strip().lower()) for x in f]

    def readStopWords(self):
        with open(self.stopWordsFile) as f:
            [self.stopWords.append(x.strip().lower()) for x in f]


    def buildDictionary(self):
        
        # Read all the stop words in a list
        self.readStopWords()

        # Read ham from ham folder and build the dictionary
        for filename in os.listdir(self.trainingHamDirectory):
            absPath = os.path.join(self.trainingHamDirectory,filename)
            self.totalDocuments += 1
            with open(absPath,errors="ignore") as f:
                for line in f:
                    line = line.lower()
                    if(line.startswith("subject") and self.ignoreSubjectLine == True):
                        continue
                    else:
                        for word in line.split():

                            if ((self.ignoreStopWords == "yes") and (word in self.stopWords)):
                                continue;

                            if word in self.ignoreWords:
                                continue

                            if word in self.ham:
                                self.ham[word] +=1
                            else:
                                self.ham[word] = 1

                            if word in self.dictionary:
                                self.dictionary[word] +=1
                            else:
                                self.dictionary[word] = 1

        # Read ham from spam folder and build the dictionary
        for filename in os.listdir(self.trainingSpamDirectory):
            absPath = os.path.join(self.trainingSpamDirectory,filename)
            self.totalSpamDocuments += 1
            self.totalDocuments+=1
            with open(absPath,errors="ignore") as f:
                for line in f:
                    line = line.lower()
                    if(line.startswith("subject") and self.ignoreSubjectLine == True):
                        continue
                    else:
                        for word in line.split():

                            if ((self.ignoreStopWords == "yes") and (word in self.stopWords)):
                                continue;

                            if word in self.ignoreWords:
                                continue

                            if word in self.spam:
                                self.spam[word] +=1
                            else:
                                self.spam[word] = 1

                            if word in self.dictionary:
                                self.dictionary[word] +=1
                            else:
                                self.dictionary[word] = 1

    def trainModel(self):
        # Calculate probability of Prior
        self.probabilityOfSpamDocument = self.totalSpamDocuments/self.totalDocuments
        self.probabilityOfHamDocument = (self.totalDocuments-self.totalSpamDocuments)/self.totalDocuments

        dictLength = len(self.dictionary)
        sumOfAllSpamWords = 0
        sumOfAllHamWords = 0

        for key,value in self.spam.items():
            sumOfAllSpamWords += value

        for key,value in self.ham.items():
            sumOfAllHamWords += value

        # Calculate likelihood
        for key,value in self.spam.items():
            self.spam[key] = (value+1)/(dictLength + sumOfAllSpamWords)

        for key,value in self.ham.items():
            self.ham[key] = (value+1)/(dictLength + sumOfAllHamWords)


    def test(self): 
        # Read file one and by one check if prediction is same os actual class
        # Starting with SPAM first
        totalNumberOfClassification = 0
        correctlyClassified = 0
        
        for filename in os.listdir(self.testingSpamDirectory):
            totalNumberOfClassification +=1
            absPath = os.path.join(self.testingSpamDirectory,filename)
            with open(absPath,errors="ignore") as f:
                probSpam = log(self.probabilityOfHamDocument)
                probHam = log(self.probabilityOfSpamDocument)
                for line in f:
                    line = line.lower()
                    if(line.startswith("subject") and self.ignoreSubjectLine == True):
                        continue
                    else:
                        for word in line.split():
                            # Ignore if stop word
                            if ((self.ignoreStopWords == "yes") and (word in self.stopWords)):
                                continue;

                            if word in self.ignoreWords:
                                continue

                            if word in self.spam:
                                probSpam = log(self.spam[word]) + probSpam
                            if word in self.ham:
                                probHam = log(self.ham[word]) + probHam
            if(probSpam <= probHam):
                    correctlyClassified +=1
            else:
                #print(absPath)
                pass



        for filename in os.listdir(self.testingHamDirectory):
            totalNumberOfClassification +=1
            absPath = os.path.join(self.testingHamDirectory,filename)
            with open(absPath,errors="ignore") as f:
                probSpam = log(self.probabilityOfHamDocument)
                probHam = log(self.probabilityOfSpamDocument)
                for line in f:
                    line = line.lower()
                    if(line.startswith("subject") and self.ignoreSubjectLine == True):
                        continue
                    else:
                        for word in line.split():
                            # Ignore if stop word
                            if ((self.ignoreStopWords == "yes") and (word in self.stopWords)):
                                continue;
                            if word in self.ignoreWords:
                                continue
                            if word in self.spam:
                                probSpam = log(self.spam[word]) + probSpam
                            if word in self.ham:
                                probHam = log(self.ham[word]) + probHam

            if(probHam <= probSpam):
                    correctlyClassified +=1
            else:
                #print(absPath)
                pass

        print("Accuracy:",(correctlyClassified/totalNumberOfClassification)*100, "%")

obj = emailClassifier(sys.argv)
obj.buildDictionary()
obj.trainModel()
obj.test()
