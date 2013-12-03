#-*- coding: utf-8 -*-

import os
from pyQClassify.libpyQClassify import Agent, QClassifyError

# Be careful - libpyQClassify.Agent is a singleton
class ColorizerAgent(Agent):	
    def __init__(self):

    	# configuration file loaded
    	self.configured = False

    	# ready to mark up
    	self.ready = False

    def getIndexFileName(self):
        return super(ColorizerAgent, self).getIndexFileName()

    def index2file(self):
    	if not self.configured:
    		raise QClassifyError('Colorizer is not configured')

        return super(ColorizerAgent, self).index2file()

    def initMarkup(self):
    	if not self.configured:
    		raise QClassifyError('Colorizer is not configured')

    	index_file = self.getIndexFileName()
    	if not os.path.exists(index_file):
    		raise OSError(2, "No such file or directory", index_file)

        res = super(ColorizerAgent, self).initMarkup()
        self.ready = True
        return res

    reinitMarkup = initMarkup    
    
    def classifyPhrase(self, phrase):
    	if not self.configured:
    		raise QClassifyError('Colorizer is not configured')

        return super(ColorizerAgent, self).classifyPhrase(phrase)
    
    def loadConfig(self, path):
    	if not os.path.exists(path):
    		raise OSError(2, "No such file or directory", path)

        res = super(ColorizerAgent, self).loadConfig(path)
        self.configured = True
        return res

    def markup(self, text):
    	if isinstance(text, unicode):
    		text = text.encode('utf8')
        return super(ColorizerAgent, self).markup(text)

    def version(self):
        return super(ColorizerAgent, self).version()

colorizer = ColorizerAgent()
