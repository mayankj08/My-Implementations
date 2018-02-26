import sys
import os
from math import log
import pandas as pd
import numpy as np
import math

class emailClassifier:
    def __init__(self,arg):
        self.trainingHamDirectory= arg[1]
        self.trainingSpamDirectory= arg[2]
        self.testingHamDirectory = arg[3]
        self.testingSpamDirectory = arg[4]
        self.ignoreStopWords = arg[5]


        self.learningRate = float(arg[6])
        self._lambda = float(arg[7])
        self.epochs = int(arg[8])

        self.stopWords = []
        self.dictionary = dict()
        self.ignoreWords = []

        self.countOfHamDoc=0

        self.stopWordsFile = "stopWords.txt"
        self.docCount = 0
        self.ignoreSubjectLine = True

        # Few class level constants
        self.IS_HAM = 1
        self.IS_SPAM = 0

    def normalize(self,df):
        result = df.copy()
        for index,feature_name in enumerate(df.columns):
            if(feature_name != '##CLASS##'):
                max_value = df.ix[:,index].max()
                min_value = df.ix[:,index].min()
                result.ix[:,index] = (df.ix[:,index] - min_value) / (max_value - min_value)
        return result

    def readStopWords(self):
        with open(self.stopWordsFile) as f:
            [self.stopWords.append(x.strip().lower()) for x in f]

    def buildDictionaryHelper(self,directory):
        for filename in os.listdir(directory):
            absPath = os.path.join(directory,filename)
            with open(absPath,errors="ignore") as f:
                self.docCount += 1
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

                            if word in self.dictionary:
                                self.dictionary[word] +=1
                            else:
                                self.dictionary[word] = 1


    def buildDataDictionary(self,directory,data,classType):
        for filename in os.listdir(directory):
            absPath = os.path.join(directory,filename)
            temp = data.iloc[self.docCount,:]
            with open(absPath,errors="ignore") as f:
                self.docCount +=1
                data.iloc[self.docCount]['##CLASS##'] = classType
                data.iloc[self.docCount]['##DUMMY##'] = 1
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

                            #(data.iloc[self.docCount])[word] +=1
                            temp[word] +=1


    def buildDictionary(self):
        # Read all the stop words in a list
        self.readStopWords()

        # Read ham from ham folder and build the dictionary
        self.buildDictionaryHelper(self.trainingHamDirectory)

        self.countOfHamDoc = self.docCount

        # Read spam from spam folder and build the dictionary
        self.buildDictionaryHelper(self.trainingSpamDirectory)

        # Now create the data dictionary. Altough the best thing would be 
        # to make it in first iteration itself but adding column and row dynamically
        # was making it too slow. So in interest of time let's read the same spam
        # and ham directory again
        temp = np.zeros(shape = (self.docCount,len(self.dictionary)+2),dtype=int)
        cols = ['##DUMMY##']
        cols = cols + list(self.dictionary.keys())  # Weird name to class variable so that it 
                                              # doesn't matched any other word in documents
        cols.append('##CLASS##')
        data = pd.DataFrame(temp,columns = cols)

        self.docCount = -1 

        self.buildDataDictionary(self.trainingHamDirectory,data,self.IS_HAM)
        self.buildDataDictionary(self.trainingSpamDirectory,data,self.IS_SPAM)

        #Normalize data to avoid overflow/underflow
        data = self.normalize(data)


        self.docCount += 1
        return data

    def trainModel(self,data):
        from random import random

        featureCount = len(self.dictionary)

        testWeight = np.zeros(shape = (1,featureCount+1),dtype=np.int)
        self.weight = pd.Series(testWeight[0],dtype = np.int)

        pr = [random()]*(self.docCount)

        #Until Convergence do
        for z in range(self.epochs):
            # For each example i
            #   Compute Pr[i] = Pr(Class = 1|Data[i],w)
            for i in range(self.docCount):
                temp = (data.iloc[i,:-1])
                newTemp = temp.reset_index(drop=True)
                newFrame = newTemp.mul(self.weight,axis=0)
                frameSum = newFrame.sum()
                num = np.exp(frameSum)
                pr[i]= np.divide(num,(np.exp(frameSum)+1))

            #Array dw[0..n] initialized to zero
            testDw = np.zeros(shape = (1,featureCount+1),dtype=int)
            dw = pd.Series(testDw[0])

            # For i=0 to n //Go over all weights
            #  For j=0 to m-1 // Go over all training exmaples
            #   dw[i] = dw[i] + data[j][i]*(data[j][n+1]-pr[j])
            # Vectoried implementation of above loop
            temp = data.iloc[:,:-1].transpose().reset_index(drop=True)
            temp1 = temp.dot(data['##CLASS##']-pr)
            dw = dw + temp1

            # For i =0 to n
            #   w[i] = w[i] + eta(dw[i]-lambda*w[i])
            self.weight = self.weight + (self.learningRate * (dw - (self._lambda*self.weight)))

    def test(self,data): 
        # Read file one and by one check if prediction is same os actual class
        # Starting with SPAM first
        totalNumberOfClassification = 0
        correctlyClassified = 0

        for filename in os.listdir(self.testingSpamDirectory):
                absPath = os.path.join(self.testingSpamDirectory,filename)

                # create pandas series to store feature for this test
                testData = np.zeros(shape = (1,len(self.dictionary)+2),dtype=int)
                tempSeries = pd.Series(testData[0],index = data.columns)
                tempSeries=tempSeries.drop(['##CLASS##'])
                tempSeries['##DUMMY##'] = 1

                with open(absPath,errors="ignore") as f:
                    totalNumberOfClassification +=1
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

                                if word not in self.dictionary:
                                    continue
                                
                                tempSeries[word] += 1
                        

                    # Now lets run testing on it
                    temp = tempSeries.reset_index(drop=True)
                    newFrame = temp.mul(self.weight)
                    frameSum = newFrame.sum()
                    outcome= np.exp(frameSum)

                    if(outcome.item() < 1):
                        correctlyClassified +=1

        for filename in os.listdir(self.testingHamDirectory):
                absPath = os.path.join(self.testingHamDirectory,filename)

                # create pandas series to store feature for this test
                testData = np.zeros(shape = (1,len(self.dictionary)+2),dtype=int)
                tempSeries = pd.Series(testData[0],index = data.columns)
                tempSeries=tempSeries.drop(['##CLASS##'])
                tempSeries['##DUMMY##'] = 1

                with open(absPath,errors="ignore") as f:
                    totalNumberOfClassification +=1
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

                                if word not in self.dictionary:
                                    continue
                                
                                tempSeries[word] += 1
                        

                    # Now lets run testing on it
                    temp = tempSeries.reset_index(drop=True)
                    newFrame = temp.mul(self.weight)
                    frameSum = newFrame.sum()
                    outcome= np.exp(frameSum)

                    if(outcome.item() >=  1):
                        correctlyClassified +=1


        print("Correctly classfied:",correctlyClassified)
        print("Total:",totalNumberOfClassification)
        print("Accuray:",(correctlyClassified/totalNumberOfClassification )*100,"%")



obj = emailClassifier(sys.argv)
data = obj.buildDictionary()
obj.trainModel(data)
obj.test(data)
