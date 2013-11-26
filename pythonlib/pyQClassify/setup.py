import os
from distutils.core import setup, Extension

PROJECT_ROOT =  '../../'

setup(	
	name 		 = 'pyQClassify', 
	version 	 = '1.0',  
	description  = 'Python wrapper for QClassify library',
	author 		 = 'Andrey Babak',
	author_email = 'a.babak@corp.mail.ru',
 	ext_modules=[
 		Extension(
 			'pyQClassify', 
 			[
 				'src/pyQClassify.cpp'
 			],
 			library_dirs=[ 		
				'/usr/local/lemmatizer/Bin',
				PROJECT_ROOT + 'libs/qclassify/.libs' ,
				'/usr/lib ',
		 	],	

		 	include_dirs=[
			 	PROJECT_ROOT,
			 	PROJECT_ROOT + 'libs',
			 	'/usr/local/lemmatizer', 
			 	'/usr/local/lemmatizer/Source',
			 	'/usr/local/lemmatizer/Source/Interfaces/cpp',
			 	'/usr/local/lemmatizer/Source/Interfaces/c',			 			 		
		 	],   

		 	libraries = [	
		 		'qclassify', 
		 		'clemmatiserrsh',
		 		'SimpleLemmatiserrsh',
		 		'Agramtabrsh',
		 		'Lemmatizerrsh',
		 		'Graphanrsh',
		 		'MorphWizardrsh',
		 		'StructDictrsh',		 		
		 		'icui18n',
		 		'icuuc',
		 		'icudata',		 		
		 		'pcre',
		 		'expat',		 		
		 	],
 		)
 	],
)