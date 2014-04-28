#-*- coding: utf-8 -*-

import os
import re
from pyQClassify.libpyQClassify import Agent, QClassifyError

class ColorizerAgent(object):   
    def __init__(self):
        self.agent = Agent()

        # configuration file loaded
        self.configured = False

        # ready to mark up
        self.ready = False

    def getIndexFileName(self):
        return self.agent.getIndexFileName()

    def index2file(self):
        if not self.configured:
            raise QClassifyError('Colorizer is not configured')

        return self.agent.index2file()

    def initMarkup(self):
        if not self.configured:
            raise QClassifyError('Colorizer is not configured')

        index_file = self.getIndexFileName()
        if not os.path.exists(index_file):
            raise OSError(2, "No such file or directory", index_file)

        res = self.agent.initMarkup()
        self.ready = True
        return res

    reinitMarkup = initMarkup    
    
    def classifyPhrase(self, phrase):
        if not self.configured:
            raise QClassifyError('Colorizer is not configured')

        return self.agent.classifyPhrase(phrase)
    
    def loadConfig(self, path):
        if not os.path.exists(path):
            raise OSError(2, "No such file or directory", path)

        res = self.agent.loadConfig(path)
        self.configured = True
        return res

    def markup(self, text):       
        is_unicode =False
        
        if isinstance(text, unicode):            
            is_unicode = True
            text = text.encode('utf8')

        marked_text = self.agent.markup(text)

        if is_unicode:
            marked_text = marked_text.decode('utf8')
        return marked_text

    def version(self):
        return self.agent.version()

colorizer = ColorizerAgent()
