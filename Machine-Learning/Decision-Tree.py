# This code implements decision tree in python using
# information gain heuristic.

import sys
import pandas as pd
import numpy as np
import logging as log

class node:
    def __init__(self,tag=None,left=None,right=None,label=None):
        self.tag=tag 
        self.left=left 
        self.right=right    
        self.label = label
        self.dropped =[]

class decisionTree:
    def __init__(self,arg):

        # init paramters for creating log
        log.basicConfig (filename = "log.txt",
                filemode = 'w',
                level = log.CRITICAL)

        self.trainingSet = arg[1]
        self.validationSet = arg[2]
        self.testSet = arg[3]
        self.toPrint = arg[4]

    def readTrainingData(self):
        train = pd.read_csv(self.trainingSet)
        return train

    def readTestingData(self):
        return (pd.read_csv(self.testSet))

    def calcEntropyForData(self,data):
        # Entropy for whole dataset
        numbersamples = data.shape[0]
        classColumn = data.shape[1]

        posExamples = data[data.iloc[:,classColumn-1]==1]  
        posExamplesSize = posExamples.shape[0]
        
        log.debug("No. of samples:%s",numbersamples);
        log.debug("No. of positive samples:%s",posExamplesSize)

        negExamplesSize = numbersamples - posExamplesSize;
       
        log.debug("No. of Negative samples:%s",negExamplesSize);
        
        pos = np.divide(posExamplesSize,numbersamples)
        neg = np.divide(negExamplesSize,numbersamples)
        
        if(neg !=0):
            temp1 = neg*np.log2(neg)
        else:
            temp1 = 0

        if(pos != 0):
            temp2 = pos*np.log2(pos)
        else:
            temp2 = 0
        entropyData = -1*(temp1 + temp2)
    
        log.debug("Entroy of complete sample:%s",entropyData)
        return entropyData

    def calcVarianceImpurityForData(self,data):
        numbersamples = data.shape[0]
        classColumn = data.shape[1]

        posExamples = data[data.iloc[:,classColumn-1]==1]  
        posExamplesSize = posExamples.shape[0]
        
        log.debug("No. of samples:%s",numbersamples);
        log.debug("No. of positive samples:%s",posExamplesSize)

        negExamplesSize = numbersamples - posExamplesSize;
       
        log.debug("No. of Negative samples:%s",negExamplesSize);
        
        pos = np.divide(posExamplesSize,numbersamples)
        neg = np.divide(negExamplesSize,numbersamples)

        varianceImpurity = pos*neg

        return varianceImpurity

    def calculateEntropyForGivenAttribute(self,attributeNo,data):
        log.debug("Calculating entropy for attribute:%s",attributeNo)
        
        numbersamples = data.shape[0]
        classColumn = data.shape[1]

        posExamples = data[data.iloc[:,attributeNo]==1]
        posExamplesSize = posExamples.shape[0]

        log.debug("No. of samples:%s",numbersamples)
        log.debug("No. of positive samples:%s",posExamplesSize)

        negExamplesSize = numbersamples - posExamplesSize;

        log.debug("No. of Negative samples:%s",negExamplesSize);

        pos = np.divide(posExamplesSize,numbersamples)
        neg = np.divide(negExamplesSize,numbersamples)

        negExamples = data[data.iloc[:,attributeNo]==0]

        entropyPos = self.calcEntropyForData(posExamples)
        entropyNeg = self.calcEntropyForData(negExamples)

        entropyAttribute = (pos*entropyPos) + (neg*entropyNeg)
        entropyAttribute = -1 * entropyAttribute

        log.debug("Entropy of attribute number %s is %s",attributeNo,entropyAttribute)
        return entropyAttribute


    def calculateVGForGivenAttribute(self,attributeNo,data):
        log.debug("Calculating VG for attribute:%s",attributeNo)
        
        numbersamples = data.shape[0]
        classColumn = data.shape[1]

        posExamples = data[data.iloc[:,attributeNo]==1]
        posExamplesSize = posExamples.shape[0]

        log.debug("No. of samples:%s",numbersamples)
        log.debug("No. of positive samples:%s",posExamplesSize)

        negExamplesSize = numbersamples - posExamplesSize;

        log.debug("No. of Negative samples:%s",negExamplesSize);

        pos = np.divide(posExamplesSize,numbersamples)
        neg = np.divide(negExamplesSize,numbersamples)

        negExamples = data[data.iloc[:,attributeNo]==0]

        VGPos = self.calcVarianceImpurityForData(posExamples)
        VGNeg = self.calcVarianceImpurityForData(negExamples)

        VGAttribute = (pos*VGPos) + (neg*VGNeg)
        VGAttribute = -1 * VGAttribute

        log.debug("VG of attribute number %s is %s",attributeNo,VGAttribute)
        return VGAttribute
    
    def getNodeForTree(self,data,drop):
        numbersamples = data.shape[0]
        totalColumnsWithoutClass = data.shape[1]-1
       
        entropySample = self.calcEntropyForData(data)
       
        indexForMaxGain = -1
        maxGain = -1; 
        # Calculate entropy for each column one by one
        for i in range(totalColumnsWithoutClass):
            if i in drop:
                continue
            entropyAttribute = self.calculateEntropyForGivenAttribute(i,data) 
            informationGain = entropySample + entropyAttribute 
            log.debug("Information gain for attribute %s is %s",i,informationGain)

            if(informationGain > maxGain):
                maxGain = informationGain
                indexForMaxGain = i
        return {'index':indexForMaxGain,'informationGain':maxGain}

    def getNodeForTreeForVG(self,data,drop):
        numbersamples = data.shape[0]
        totalColumnsWithoutClass = data.shape[1]-1
       
        VGSample = self.calcVarianceImpurityForData(data)
       
        indexForMaxGain = -1
        maxGain = -1; 
        for i in range(totalColumnsWithoutClass):
            if i in drop:
                continue
            VGAttribute = self.calculateVGForGivenAttribute(i,data) 
            VG = VGSample + VGAttribute 
            log.debug("VG for attribute %s is %s",i,VG)

            if(VG> maxGain):
                maxGain = VG 
                indexForMaxGain = i
        return {'index':indexForMaxGain,'VG':maxGain}

    def createTree(self,root,data):
        # Deal with postive side of root
        posData = data[data.iloc[:,root.tag]==1]
       
        if(posData.shape[0] != 0):
            result = self.getNodeForTree(posData,root.dropped)                
            posIndex = result['index']
            informationGain = result['informationGain']

            newNode = node()

            if(informationGain == 0):
                newNode.tag = -1
            else:
                newNode.tag = posIndex
            
            if(newNode.tag != -1):
                log.debug("Created new node with element:%s 1 and root %s",data.columns[posIndex],data.columns[root.tag])
            else:
                log.debug("Created new node with element:%s 1 and root %s","-1",data.columns[root.tag])
            newNode.dropped = list(root.dropped)
            newNode.dropped.append(posIndex)
          
            if(informationGain <= 0):
                root.right = newNode
                classColumn = posData.shape[1]
                posExamples = posData[posData.iloc[:,classColumn-1]==1].shape[0]
                negExamples = posData[posData.iloc[:,classColumn-1]==0].shape[0]
                
                if(posExamples > negExamples):
                    newNode.label = 1
                elif(posExamples < negExamples):
                    newNode.label = 0
                else:
                    newNode.label = 1 #Some random Value
            elif(posIndex != -1):
                root.right = newNode
                self.createTree(newNode,posData);                
            else:
                assert False

        # Now start  with negative side of root
        negData = data[data.iloc[:,root.tag]==0]

        if(negData.shape[0] != 0):
            result = self.getNodeForTree(negData,root.dropped)
        
            negIndex = result['index']
            informationGain = result['informationGain']

            log.debug("informationGain as returned is:%s",informationGain)

            newNode = node()

            if(informationGain == 0):
                newNode.tag = -1
            else:
                newNode.tag = negIndex
           
            if(newNode.tag != -1):
                log.debug("Created new node with element:%s 0 and root %s",data.columns[negIndex],data.columns[root.tag])
            else:
                log.debug("Created new node with element:%s 0 and root %s","-1",data.columns[root.tag])
            
            newNode.dropped = list(root.dropped)
            newNode.dropped.append(negIndex)


            if(informationGain <= 0  ):
                root.left = newNode
                classColumn = negData.shape[1]
                posExamples = negData[negData.iloc[:,classColumn-1]==1].shape[0]
                negExamples = negData[negData.iloc[:,classColumn-1]==0].shape[0]
                if(posExamples > negExamples):
                   newNode.label = 1
                elif(posExamples < negExamples):
                    newNode.label = 0
                else:
                    newNode.label = 1 #Some random Value
            elif(negIndex != -1):
                root.left = newNode
                self.createTree(newNode,negData);                
            else:
                assert False

    def createTreeForVG(self,root,data):
        # Deal with postive side of root
        posData = data[data.iloc[:,root.tag]==1]
       
        if(posData.shape[0] != 0):
            result = self.getNodeForTreeForVG(posData,root.dropped)                
            posIndex = result['index']
            VG= result['VG']

            newNode = node()

            if(VG == 0):
                newNode.tag = -1
            else:
                newNode.tag = posIndex
            
            if(newNode.tag != -1):
                log.debug("Created new node with element:%s 1 and root %s",data.columns[posIndex],data.columns[root.tag])
            else:
                log.debug("Created new node with element:%s 1 and root %s","-1",data.columns[root.tag])
            newNode.dropped = list(root.dropped)
            newNode.dropped.append(posIndex)
          
            if(VG <= 0):
                root.right = newNode
                classColumn = posData.shape[1]
                posExamples = posData[posData.iloc[:,classColumn-1]==1].shape[0]
                negExamples = posData[posData.iloc[:,classColumn-1]==0].shape[0]
                
                if(posExamples > negExamples):
                    newNode.label = 1
                elif(posExamples < negExamples):
                    newNode.label = 0
                else:
                    newNode.label = 1 #Some random Value
            elif(posIndex != -1):
                root.right = newNode
                self.createTree(newNode,posData);                
            else:
                assert False

        
        
        # Now start  with negative side of root
        negData = data[data.iloc[:,root.tag]==0]

        if(negData.shape[0] != 0):
            result = self.getNodeForTreeForVG(negData,root.dropped)
        
            negIndex = result['index']
            VG= result['VG']

            log.debug("Vg as returned is:%s",VG)

            newNode = node()

            if(VG == 0):
                newNode.tag = -1
            else:
                newNode.tag = negIndex
           
            if(newNode.tag != -1):
                log.debug("Created new node with element:%s 0 and root %s",data.columns[negIndex],data.columns[root.tag])
            else:
                log.debug("Created new node with element:%s 0 and root %s","-1",data.columns[root.tag])
            
            newNode.dropped = list(root.dropped)
            newNode.dropped.append(negIndex)


            if(VG <= 0  ):
                root.left = newNode
                classColumn = negData.shape[1]
                posExamples = negData[negData.iloc[:,classColumn-1]==1].shape[0]
                negExamples = negData[negData.iloc[:,classColumn-1]==0].shape[0]
                if(posExamples > negExamples):
                   newNode.label = 1
                elif(posExamples < negExamples):
                    newNode.label = 0
                else:
                    newNode.label = 1 #Some random Value
            elif(negIndex != -1):
                root.left = newNode
                self.createTree(newNode,negData);                
            else:
                assert False

    def traverseTree(self,data,root,level):
        if(root.left.tag == -1):
            print('|'*level,data.columns[root.tag],"= 0 :" +str(root.left.label),sep = "")
        else:
            print('|'*level,data.columns[root.tag],"= 0",sep = "")
            self.traverseTree(data,root.left,level+1)
        
        if(root.right.tag == -1):
            print('|'*level,data.columns[root.tag],"= 1 :" +str(root.right.label),sep="")
        
        else:
            print('|'*level,data.columns[root.tag],"= 1",sep = "")
            self.traverseTree(data,root.right,level+1)

    def informationGainHeuristic(self,train):
        root = node()
        result = self.getNodeForTree(train,root.dropped)
        index = result['index'] #TODO: What if information gain is 0 here itself?
        root.tag = index

        log.debug("Created root with element %s",train.columns[index])
        
        root.dropped.append(index)
        self.createTree(root,train)       

        if(self.toPrint.lower() == "yes"):
            print("Tree Structure for Information Gain Heuristici:")
            self.traverseTree(train,root,0)
        return root

    def varianceImpurityHeuristic(self,train):
        root = node()
        result = self.getNodeForTreeForVG(train,root.dropped)
        index = result['index'] #TODO: What if Variance gain is 0 here itself?
        root.tag = index

        log.debug("Created root with element %s",train.columns[index])
        
        root.dropped.append(index)
        self.createTreeForVG(root,train)

        if(self.toPrint.lower() == "yes"):
            print("Tree Structure for Variance Impurity Heuristic:")
            self.traverseTree(train,root,0)
        return root

    def runTestCase(self,root,test):

        currNode = root
        while(1):
            if(currNode.tag == -1):
                break

            currIndex = currNode.tag
            valueOfCurrentIndex = test.iloc[currIndex]
            if(valueOfCurrentIndex == 1):
                currNode = currNode.right
            elif(valueOfCurrentIndex == 0):
                currNode = currNode.left
            else:
                assert False
        
        
        classVar = test.shape[0]
        classVar = classVar -1 
        if(currNode.label == test.iloc[classVar]):
            return True
        else:
            return False

    def test(self,root,tests):
        testCasesRan = 0
        testCasesPassed = 0
        for index,test in tests.iterrows():
            result = self.runTestCase(root,test)
            testCasesRan = testCasesRan +1
            if(result == True):
                testCasesPassed +=1
        accuracy = testCasesPassed/testCasesRan
        return accuracy

    def run(self):
        # Let's start with training our model
        train = self.readTrainingData()
        rootIG = self.informationGainHeuristic(train)
        rootVI = self.varianceImpurityHeuristic(train)
        # After training lets start with testing  

        test = self.readTestingData()        
        acc = self.test(rootIG,test)
        print("Accuracy for Information Heuristic model is:",acc)

        acc1 = self.test(rootVI,test)
        print("Accuracy for Variance impurity model is:",acc1)



tree = decisionTree(sys.argv);
tree.run()
