#-*- coding: utf-8 -*-

from distutils.core import setup, Extension

PROJECT_ROOT =  '../../'

pyQClassify_module = Extension(
	'_pyQClassify', 
	sources=[		
		'pyQClassify.i',			
		'pyQClassify.cpp',
	], 
	swig_opts=['-c++',],	
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

setup(	
	name 		 = 'pyQClassify', 
	version 	 = '2.0.0',
	description  = 'Python wrapper for QClassify library',
	author 		 = 'Andrey Babak',
	author_email = 'a.babak@corp.mail.ru',
	py_modules = ["pyQClassify"],
    ext_modules=[pyQClassify_module], 	
)
