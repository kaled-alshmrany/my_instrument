#!/usr/bin/env python3
import os.path
import xml.etree.ElementTree as ET # To parse XML
import os
import argparse
import shlex
import subprocess
import signal
import time
import sys
import resource
import re
import zipfile
import traceback
#from time import process_time 
#from time import process_time_ns
import hashlib

from threading import Thread
#from threading import Lock
from queue import Empty

try:
	import queue
except ImportError:
	import Queue as queue
from random import randrange
import random
import string
from datetime import datetime

FUSEBMC_VERSION = 'v.4.1.3'
# Start time for this script
start_time = time.time()
#start_time=process_time()
property_file_content = ""
category_property = 0
benchmark=''
property_file=''
witness_file_name=''
toWorkSourceFile=''
arch=''

__graphml_base__ = '{http://graphml.graphdrawing.org/xmlns}'
__graph_tag__ = __graphml_base__ + 'graph'
__edge_tag__ = __graphml_base__ + 'edge'
__data_tag__ = __graphml_base__ + 'data'
WRAPPER_OUTPUT_DIR ='./fusebmc_output/'

__testSuiteDir__ = "test-suite/"
META_DATA_FILE = __testSuiteDir__ + "/metadata.xml"
TESTCASE_FILE_FOR_COVER_ERROR=__testSuiteDir__ + "/testcase_1_ES.xml"
TESTCASE_FILE_FOR_COVER_ERROR_RANDOM = __testSuiteDir__ + "/testcase_1_Fuzzer.xml"
TESTCASE_FILE_FOR_COVER_ERROR_RANDOM2 = __testSuiteDir__ + "/testcase_2_Fuzzer.xml"
TESTCASE_FILE_FOR_COVER_ERROR_RANDOM3 = __testSuiteDir__ + "/testcase_3_Fuzzer.xml"

INSTRUMENT_OUTPUT_DIR = './fusebmc_instrument_output/'
INSTRUMENT_OUTPUT_FILE = './fusebmc_instrument_output/instrumented.c'
INSTRUMENT_OUTPUT_FILE_OBJ = './fusebmc_instrument_output/instrumented.o'
INSTRUMENT_OUTPUT_GOALS_FILE = './fusebmc_instrument_output/goals.txt'
INSTRUMENT_OUTPUT_GOALS_DIR = './fusebmc_instrument_output/goals_output/'



TEST_SUITE_DIR_ZIP = ''
MAX_NUM_OF_LINES_OUT = 50
MAX_NUM_OF_LINES_ERRS = 50
SHOW_ME_OUTPUT = not True
IS_DEBUG = True
IS_TIME_OUT_ENABLED = True
MUST_COMPILE_INSTRUMENTED = False
MUST_GENERATE_RANDOM_TESTCASE = True
MUST_ADD_EXTRA_TESTCASE = True
MUST_APPLY_TIME_PER_GOAL = True
MUST_APPLY_LIGHT_INSTRUMENT_FOR_BIG_FILES = True

MEM_LIMIT_BRANCHES_ESBMC = 10 # giga ; Zero or negative means unlimited. 
MEM_LIMIT_BRANCHES_MAP2CHECK = 1000 # miga

MEM_LIMIT_ERROR_CALL_ESBMC = 10
MEM_LIMIT_ERROR_CALL_MAP2CHECK = 1000 # miga
MEM_LIMIT_MEM_OVERFLOW_REACH_TERM_MAP2CHECK = 1000 # miga

MEM_LIMIT_ESBMC = 10 # GiGA: Memory limit for other properties.

BIG_FILE_LINES_COUNT = 8000

EXTRA_TESTCASE_COUNT = 6

esbmc_path = "./esbmc "
#esbmc_path = '/home/kaled/sdb1/esbmc_work/release/bin/esbmc '

#map2check_path = os.path.abspath('./map2check/map2check ')
map2check_path = os.path.abspath('./map2check-fuzzer/map2check ')
map2checkTime = 150 #seconds
map2checkTimeErrorCall_Symex = 40 #seconds
map2checkTimeErrorCall_Fuzzer = 150 #seconds
map2checkTimeMemOverflowReachTerm_Fuzzer = 150 #seconds
MUST_RUN_MAP_2_CHECK_FOR_ERROR_CALL_SYMEX = False # we can change it.
MUST_RUN_MAP_2_CHECK_FOR_ERROR_CALL_FUZZER = True # we can change it.
MUST_RUN_MAP_2_CHECK_FOR_BRANCHES = True # we can change it.
MUST_RUN_MAP_2_CHECK_FOR_MEM_OVERFLOW_REACH_TERM = True # we can change it.
map2checkTimeForBranches = 70 #seconds

map2checkWitnessFile='' # will be set later in the wrapper output

COV_TEST_EXE = './val_testcov/testcov/bin/testcov'
#COV_TEST_EXE = './test-suite-validator/bin/testcov'
FUSEBMC_INSTRUMENT_EXE_PATH = './FuSeBMC_inustrument/FuSeBMC_instrument'

C_COMPILER = 'gcc'
#COV_TEST_PARAMS = ['--no-runexec','--use-gcov','-64']
#COV_TEST_PARAMS= ['--no-runexec','--use-gcov','--cpu-cores','0', '--verbose', '--no-plots','--reduction','BY_ORDER','--reduction-output','test-suite']
#COV_TEST_PARAMS= ['--no-runexec', '--no-isolation', '--memlimit', '6GB', '--timelimit-per-run', '3', '--cpu-cores', '0', '--verbose', '--no-plots','--reduction', 'BY_ORDER','--reduction-output','test-suite']
COV_TEST_PARAMS = ['--no-isolation','--memlimit', '6GB','--timelimit-per-run', '3', '--cpu-cores', '0','--verbose','--no-plots','--reduction', 'BY_ORDER', '--reduction-output','test-suite']
RUN_COV_TEST = True
RUN_CPA_CHECKER = False
CPA_CHECKER_EXE = '/home/kaled/sdb1/FuSeBMC/CPAchecker-2.0-unix/scripts/cpa.sh'
time_out_s = 890 # 890 seconds 
time_for_zipping_s = 10 # the required time for zipping folder; Can Zero ??
is_ctrl_c = False
remaining_time_s = 0

goals_count = 0
#fdebug = None

hasInputInTestcase = False # don't change it.
map2CheckInputCount = 0
lastInputInTestcaseCount = 0

mustRunTwice = True # You can change it.
mustRunTwice_MemOverflowReachTerm = True # You can change it.
runNumber = 1

FuSeBMC_GoalTracerLib_Enabled = True 
mustRunGoalsBasedOnType = True

FuSeBMCFuzzerLib_ERRORCALL_ENABLED = True
FuSeBMCFuzzerLib_ERRORCALL_TIMEOUT = 100 # second

FuSeBMCFuzzerLib_COVERBRANCHES_ENABLED =  True
FuSeBMCFuzzerLib_COVERBRANCHES_TIMEOUT = 60 # second
FuSeBMCFuzzerLib_COVERBRANCHES_DONE = False # Don't change it.
FuSeBMCFuzzerLib_COVERBRANCHES_MAX_TESTCASE_SIZE_BYTES = -1 # Don't change it.
FuSeBMCFuzzerLib_COVERBRANCHES_NUM_OF_GENERATED_TESTCASES = 0 #Don't change it; The number og generated testcases from cover-Branches.
FuSeBMCFuzzerLib_COVERBRANCHES_NUM_OF_GENERATED_TESTCASES_TO_RUN_AFL = 3 # You can change.
FuSeBMCFuzzerLib_COVERBRANCHES_BYTES_PER_NUMBER = 8 # You can change
FuSeBMCFuzzerLib_COVERBRANCHES_SEED_DIR ='' #Don't change
FuSeBMCFuzzerLib_COVERBRANCHES_COVERED_GOALS_FILE = '' #Don't change
FuSeBMCFuzzerLib_COVERBRANCHES_DEFAULT_AFL_MIN_LENGTH = 36 # bytes ; you can change it ; -1: different lengths

AFL_HOME_PATH = os.path.abspath('./AFL-2.57b/')
fuSeBMC_run_id =''

#def logText(txt):
#	global fdebug
	#fdebug = open(WRAPPER_OUTPUT_DIR + '/fdebug.txt','a')
	#fdebug.write(txt);
	#fdebug.close()
important_outs_by_ESBMC=["Timed out","Out of memory","Chosen solver doesn\'t support floating-point numbers",
						"dereference failure: forgotten memory","dereference failure: invalid pointer",
						"dereference failure: Access to object out of bounds", "dereference failure: NULL pointer",
						"dereference failure: invalidated dynamic object", "dereference failure: invalidated dynamic object freed", 
						"dereference failure: invalid pointer freed","dereference failure: free() of non-dynamic memory","array bounds violated",
						"Operand of free must have zero pointer offset", "VERIFICATION FAILED", "unwinding assertion loop", 
						" Verifier error called", "VERIFICATION SUCCESSFUL"]

#important_outs_by_ESBMC=["Chosen solver doesn\'t support floating-point numbers"]

lineNumberForNonDetCallsLst = None
#lineNumberForNonDetCallsLst_Map2check = None

class TColors:
	HEADER = '\033[95m'
	OKBLUE = '\033[94m'
	OKGREEN = '\033[92m'
	WARNING = '\033[93m'
	FAIL = '\033[91m'
	ENDC = '\033[0m'
	BOLD = '\033[1m'
	UNDERLINE = '\033[4m'

class GoalType:
	IF = 1
	ELSE = 2
	FOR = 3
	CXX_FOR_RANGE = 4
	DO_WHILE = 5
	WHILE = 6
	COMPOUND = 7
	LOOP = 8
	END_OF_FUNCTION = 9
	AFTER_LOOP = 10
	EMPTY_ELSE = 11
	NONE = 12
	
	@staticmethod
	def GetAsString(goalType):
		if goalType == GoalType.IF: return 'IF'
		if goalType == GoalType.ELSE: return 'ELSE'
		if goalType == GoalType.FOR: return 'FOR'
		if goalType == GoalType.CXX_FOR_RANGE : return 'CXX_FOR_RANGE'
		if goalType == GoalType.DO_WHILE: return 'DO_WHILE'
		if goalType == GoalType.WHILE: return 'WHILE'
		if goalType == GoalType.COMPOUND: return 'COMPOUND'
		if goalType == GoalType.LOOP: return 'LOOP'
		if goalType == GoalType.END_OF_FUNCTION : return 'END_OF_FUNCTION'
		if goalType == GoalType.AFTER_LOOP: return 'AFTER_LOOP'
		if goalType == GoalType.EMPTY_ELSE : return 'EMPTY_ELSE'
		if goalType == GoalType.NONE: return 'NONE'

class GoalInfo:
	def __init__(self,goal = 0, goalType = GoalType.NONE, depth = -1):
		self.goal = goal
		self.goalType = goalType
		self.depth = depth
		
	def toString(self):
		s = '(GOAL_' + str(self.goal)
		if self.goalType != GoalType.NONE : s += ',' + GoalType.GetAsString(self.goalType)
		if(self.depth != -1) : s += ',' + str(self.depth)
		s += ')'
		return s
	def __str__(self):
		return self.toString()

class GoalSorting:
	SEQUENTIAL = 1
	TYPE_THEN_DEPTH = 2
	DEPTH_THEN_TYPE = 3

#goalSorting = GoalSorting.SEQUENTIAL
#goalSorting = GoalSorting.TYPE_THEN_DEPTH
goalSorting = GoalSorting.DEPTH_THEN_TYPE

goalSorting_IF_first = False # valid by DEPTH_THEN_TYPE
goalSorting_EMPTYELSE_last = True # valid by DEPTH_THEN_TYPE
goalSorting_AFTERLOOP_last = True # valid by DEPTH_THEN_TYPE

class InputType:
	unkonwn = 0,
	_char = 1
	_uchar = 2
	_short = 3
	_ushort = 4
	_int = 5
	_uint = 6
	_long = 7
	_ulong = 8
	_longlong = 9
	_ulonglong = 10
	_float = 11
	_double = 12
	_bool = 13
	_string = 14
	@staticmethod
	def GetInputTypeForNonDetFunc(func):
		if func == '__VERIFIER_nondet_char': return InputType._char
		if func == '__VERIFIER_nondet_uchar' or func == '__VERIFIER_nondet_u8' or \
			func == '__VERIFIER_nondet_unsigned_char':
			return InputType._uchar
		if func == '__VERIFIER_nondet_short': return InputType._short
		if func == '__VERIFIER_nondet_ushort' or func == '__VERIFIER_nondet_u16':
			return InputType._ushort
		if func == '__VERIFIER_nondet_int': return InputType._int
		if func == '__VERIFIER_nondet_uint' or func == '__VERIFIER_nondet_size_t' or \
			func == '__VERIFIER_nondet_u32' or func == '__VERIFIER_nondet_U32' or func == '__VERIFIER_nondet_unsigned':
			return InputType._uint
		if func == '__VERIFIER_nondet_long': return InputType._long
		if func == '__VERIFIER_nondet_ulong' or func == '__VERIFIER_nondet_pointer':
			return InputType._ulong
		if func == '__VERIFIER_nondet_longlong': return InputType._longlong
		if func == '__VERIFIER_nondet_ulonglong': return InputType._ulonglong
		if func == '__VERIFIER_nondet_float': return InputType._float
		if func == '__VERIFIER_nondet_double': return InputType._double
		if func == '__VERIFIER_nondet_bool': return InputType._bool
		if func == '__VERIFIER_nondet_string': return InputType._string
		if IS_DEBUG:
			print(TColors.FAIL,'func:', func , 'has no type...',TColors.ENDC)
			sys.exit(0)
		return InputType.unkonwn
	@staticmethod
	def GetAs_C_DataType(inputType):
		if inputType == InputType.unkonwn: return ''
		if inputType == InputType._char: return 'char'
		if inputType == InputType._uchar: return 'unsigned char'
		if inputType == InputType._short : return 'short'
		if inputType == InputType._ushort : return 'unsigned short'
		if inputType == InputType._int : return 'int'
		if inputType == InputType._uint : return 'unsigned int'
		if inputType == InputType._long: return 'long'
		if inputType == InputType._ulong : return 'unsigned long'
		if inputType == InputType._longlong : return 'long long'
		if inputType == InputType._ulonglong : return 'unsigned long long'
		if inputType == InputType._float : return 'float'
		if inputType == InputType._double: return 'double'
		if inputType == InputType._bool : return 'bool'
		if inputType == InputType._string : return 'string'

	@staticmethod
	def GeSizeForDataType(inputType,p_arch):
		if inputType == InputType.unkonwn: return FuSeBMCFuzzerLib_COVERBRANCHES_BYTES_PER_NUMBER
		if inputType == InputType._char: return 1
		if inputType == InputType._uchar: return 1
		if inputType == InputType._short : return 2
		if inputType == InputType._ushort : return 2
		if inputType == InputType._int : return 4
		if inputType == InputType._uint : return 4
		if inputType == InputType._long:
			if p_arch == 32: return 4
			else : return 8
		if inputType == InputType._ulong : 
			if p_arch == 32: return 4
			else : return 8
		if inputType == InputType._longlong : return 8
		if inputType == InputType._ulonglong : return 8
		if inputType == InputType._float : return 4
		if inputType == InputType._double: return 8
		if inputType == InputType._bool : return 1
		if inputType == InputType._string :
			if IS_DEBUG:
				print(TColors.FAIL,'dont call GeSizeForDataType for String..',TColors.ENDC)
				sys.exit(0)
			return FuSeBMCFuzzerLib_COVERBRANCHES_BYTES_PER_NUMBER
		
		return FuSeBMCFuzzerLib_COVERBRANCHES_BYTES_PER_NUMBER
	@staticmethod
	def IsUnSigned(inputType):
		return inputType == InputType._uchar or inputType == InputType._uint \
			or inputType == InputType._ushort or inputType == InputType._ulong \
			or inputType == InputType._ulonglong

#See http://eyalarubas.com/python-subproc-nonblock.html
class NonBlockingStreamReader:

	def __init__(self, stream):
		'''
		stream: the stream to read from.
				Usually a process' stdout or stderr.
		'''

		self._s = stream
		self._q = queue.Queue()
		self.exception = None
		self.isEndOfStream=True
		#self.mutex = Lock()
		self._t = Thread(target=self._populateQueue, args = ())
		self._t.daemon = True
		self._t.start() #start collecting lines from the stream

	def _populateQueue(self):
		''' Collect lines from 'stream' and put them in 'quque'. '''
		self.isEndOfStream = False
		while True:
			line = self._s.readline()
			if line:
				#self.mutex.acquire()
				line_de=line.decode('utf-8').rstrip()
				#print('line_de',line_de)
				self._q.put(line_de)
				#self.mutex.release()
			else:
				#raise UnexpectedEndOfStream
				self.isEndOfStream = True
				break

	def readline(self, timeout = None):
		data = None
		try:
			#self.mutex.acquire()
			#https://docs.python.org/3/library/queue.html
			data= self._q.get(block = timeout is not None,timeout = timeout)
			if data : self._q.task_done()
			#data= self._q.get()
			
		except Empty as empt:
			#self._q.mutex.release()
			self.exception = empt
			data = None
		finally:
			#self.mutex.release()
			pass
		return data
	def hasMore(self):
		if self.isEndOfStream == True and self._q.empty() == True:
			return False
		return True

class UnexpectedEndOfStream(Exception): pass

class MyTimeOutException(Exception):
	pass
	
def IsTimeOut(must_throw_ex = False):
	global is_ctrl_c
	global time_out_s
	global start_time
	global remaining_time_s
	global fdebug
	global IS_TIME_OUT_ENABLED
	#global lasttime
	if is_ctrl_c is True:
		raise KeyboardInterrupt()
	if IS_TIME_OUT_ENABLED == False: return False

	#curr_gp_times = os.times()
	#user=curr_gp_times[0]
	#system=curr_gp_times[1]
	#children_user=curr_gp_times[2]
	#children_system=curr_gp_times[3]
	#exec_time_s = int(user + system + children_user + children_system)
	
	#if(exec_time_s != lasttime):
	#	lasttime=exec_time_s
	#	logText('exec_time_s=' + str(exec_time_s) + ' time_out_s:' + str(time_out_s) + '\n')

	end_time = time.time()
	wall_exec_time_s=(end_time - start_time)
	wall_remaining_time_s= time_out_s - wall_exec_time_s
	remaining_time_s = wall_remaining_time_s
	isWallTimeout = False
	if(wall_exec_time_s >= time_out_s):
		isWallTimeout = True
		if must_throw_ex:
			raise MyTimeOutException()

	#RUSAGE_CHILDREN
	ruAfter = resource.getrusage(resource.RUSAGE_SELF)
	rChild = resource.getrusage(resource.RUSAGE_CHILDREN)
	cpu_exec_time_s = (ruAfter.ru_utime + ruAfter.ru_stime + rChild.ru_utime + rChild.ru_stime)

	cpu_remaining_time_s = time_out_s - cpu_exec_time_s
	if cpu_remaining_time_s > remaining_time_s :
		remaining_time_s = cpu_remaining_time_s

	isCpuTimeout = False
	if(cpu_exec_time_s >= time_out_s):
		isCpuTimeout = True
		if must_throw_ex:
			raise MyTimeOutException()

	return isWallTimeout or isCpuTimeout
def GetSH1ForFile(fil):
	BUF_SIZE = 32768
	sha1 = hashlib.sha1()
	with open(fil, 'rb') as f:
		while True:
			data = f.read(BUF_SIZE)
			if not data:
				break
			sha1.update(data)
		return sha1.hexdigest()
	return ''

#https://stackoverflow.com/questions/10501247/best-way-to-generate-random-file-names-in-python?lq=1
def GenerateRondomFileName():
	return ''.join(random.choice(string.ascii_letters) for _ in range(25))

def AddFileToArchive(full_file_name, zip_file_name):
	if not os.path.isfile(full_file_name): return
	os.makedirs(os.path.dirname(os.path.abspath(zip_file_name)), exist_ok=True)
	appendOrCreate='w'
	if os.path.isfile(zip_file_name): appendOrCreate='a'
	zipf = zipfile.ZipFile(zip_file_name, appendOrCreate , zipfile.ZIP_DEFLATED)
	zipf.write(os.path.abspath(full_file_name),os.path.basename(os.path.abspath(full_file_name)))
	zipf.close()
	
def ZipDir(path, zip_file_name):
	os.makedirs(os.path.dirname(os.path.abspath(zip_file_name)), exist_ok=True)
	#RemoveFileIfExists(zip_file_name)
	zipf = zipfile.ZipFile(zip_file_name, 'w', zipfile.ZIP_DEFLATED)
	for root, dirs, files in os.walk(os.path.abspath(path)):
		for file in files:
			zipf.write(os.path.join(root,file),file)
	zipf.close()

def MakeFolderEmptyORCreate(flder):
	if os.path.isdir(flder):
		for file_loop in os.listdir(flder):
			file_to_del=os.path.join(flder, file_loop)
			if os.path.isfile(file_to_del):
				os.remove(file_to_del)
	else:
		os.mkdir(flder)

def RemoveFileIfExists(fil):
	if os.path.isfile(fil): os.remove(fil)

class AssumptionHolder(object):
	"""Class to hold line number and assumption from ESBMC Witness."""

	def __init__(self, line, assumption):
		"""
		Default constructor.

		Parameters
		----------
		line : unsigned
			Line Number from the source file
		assumption : str
			Assumption string from ESBMC.
		"""
		assert(line >= 0)
		assert(len(assumption) > 0)
		self.line = line
		self.assumption = assumption
		#self.varName = varName
		#self.varVal = varVal

	def __str__(self):
		return "AssumptionInfo: LINE: {0}, ASSUMPTION: {1}".format(self.line, self.assumption)
	
	def debugInfo(self):
		"""Print info about the object"""
		print("AssumptionInfo: LINE: {0}, ASSUMPTION: {1}".format(
			self.line, self.assumption))


class AssumptionParser(object):
	"""Class to parse a witness file generated from ESBMC and create a Set of AssumptionHolder."""

	def __init__(self, witness,pIsFromMap2Check = False):
		"""
		Default constructor.

		Parameters
		----------

		witness : str
			Path to witness file (absolute/relative)
		"""
		assert(os.path.isfile(witness))
		self.__xml__ = None
		self.assumptions = list()
		self.__witness__ = witness
		self.isFromMap2Check = pIsFromMap2Check

	def __openwitness__(self):
		"""Parse XML file using ET"""
		self.__xml__ = ET.parse(self.__witness__).getroot()

	def parse(self):
		""" Iterates over all elements of GraphML and extracts all Assumptions """
		if self.__xml__ is None:
			try:
				self.__openwitness__()
			except:
				if IS_DEBUG :
					print(TColors.FAIL,'Cannot parse file:', self.__witness__,TColors.ENDC)
		if self.__xml__ is None:
			return

		graph = self.__xml__.find(__graph_tag__)
		for node in graph:
			try:
				if(node.tag == __edge_tag__):
					startLine = 0
					assumption = ""
					for data in node:
						if data.attrib['key'] == 'startline':
							startLine = int(data.text)
						elif data.attrib['key'] == 'assumption':
							assumption = data.text
					if assumption != "":
						#print('assumption',assumption)
						#if self.isFromMap2Check:
						#	assum_l,assum_r= assumption.split(' == ')
						#	if assum_l.find('\\')== 0: assum_l = assum_l.replace('\\','',1)
						#else:
						#	assum_l,assum_r= assumption.split('=')
						#	assum_r = assum_r.replace(';','',1)
						#	if assum_r[-1] == "f" or assum_r[-1] == "l": 
						#		assum_r = assum_r[:-1]

						#TODO: handle f and l
						#_, right = pAssumptionHolder.assumption.split("=")
						
						#assum_l = assum_l.strip()
						#assum_r = assum_r.strip()		
						#print('assum_l',assum_l,len(assum_l), 'assum_r',assum_r,len(assum_r))
						self.assumptions.append(AssumptionHolder(startLine, assumption))
						#assumption : n = 2;
						#print('ass',assumption,'start',startLine)
			except:
				if IS_DEBUG :
					print(TColors.FAIL,'Cannot parse node:',TColors.ENDC)

	def debugInfo(self):
		"""Print current info about the object"""
		print("XML: {0}".format(self.__witness__))
		print("ET: {0}".format(self.__xml__))
		for assumption in self.assumptions:
			assumption.debugInfo()

def WriteMetaDataFromWrapper():
	now = datetime.now()
	with open(META_DATA_FILE, 'w') as meta_f:
		meta_f.write('<?xml version="1.0" encoding="UTF-8" standalone="no"?><!DOCTYPE test-metadata PUBLIC "+//IDN sosy-lab.org//DTD test-format test-metadata 1.0//EN" "https://sosy-lab.org/test-format/test-metadata-1.0.dtd">')
		meta_f.write('<test-metadata>')
		meta_f.write('<entryfunction>main</entryfunction>')
		meta_f.write('<specification>'+property_file_content.strip()+'</specification>')
		meta_f.write('<sourcecodelang>'+'C'+'</sourcecodelang>')
		_arch ='32bit'
		if arch == 64: _arch='64bit'
		meta_f.write('<architecture>'+_arch+'</architecture>')
		#meta_f.write('<creationtime>2020-07-27T21:33:51.462605</creationtime>')
		meta_f.write('<creationtime>'+now.strftime("%Y-%m-%dT%H:%M:%S.%f")+'</creationtime>')
		meta_f.write('<programhash>'+GetSH1ForFile(benchmark)+'</programhash>')
		meta_f.write('<producer>FuSeBMC ' + FUSEBMC_VERSION + '</producer>')
		meta_f.write('<programfile>'+benchmark+'</programfile>')
		meta_f.write('</test-metadata>')

class MetadataParser(object):
	"""Class to parse a witness file generated from ESBMC and extract all metadata from it."""

	def __init__(self, witness):
		"""
		Default constructor.

		Parameters
		----------

		witness : str
			Path to witness file (absolute/relative)
		"""
		assert(os.path.isfile(witness))
		self.__xml__ = None
		self.metadata = {}
		self.__witness__ = witness

	def __openwitness__(self):
		"""Parse XML file using ET"""
		self.__xml__ = ET.parse(self.__witness__).getroot()

	def parse(self):
		""" Iterates over all elements of GraphML and extracts all Metadata """
		if self.__xml__ is None:
			self.__openwitness__()
		graph = self.__xml__.find(__graph_tag__)
		for node in graph:			
			if(node.tag == __data_tag__):
				self.metadata[node.attrib['key']] = node.text


class NonDeterministicCall(object):
	def __init__(self,value):
		"""
		Default constructor.

		Parameters
		----------
		value : str
			String containing value from input		
		"""
		self.inputType = InputType.unkonwn
		assert(len(value) > 0)
		self.value = value

	def __str__(self):
		return "NonDeterministicCall: VALUE: {0}".format(self.value)
	@staticmethod
	def extract_byte_little_endian(value):
		"""
		Converts an byte_extract_little_endian(%d, %d) into an value

				Parameters
		----------
		value : str
			Nondeterministic assumption
		"""
		PATTERN = 'byte_extract_little_endian\((.+), (.+)\)'
		INT_BYTE_SIZE = 4
		match = re.search(PATTERN, value)
		if match == None:
			return value
		number = match.group(1)
		index = match.group(2)

		byte_value = (int(number)).to_bytes(INT_BYTE_SIZE, byteorder='little', signed=False)
		return str(byte_value[int(index)])


	@staticmethod
	def fromAssumptionHolder(pAssumptionHolder, isFromMap2Check = False):
		"""
		Converts an Assumption (that is nondet, this function will not verify this) into a NonDetermisticCall

		Parameters
		----------
		pAssumptionHolder : AssumptionHolder
			Nondeterministic assumption
		"""
		#value = NonDeterministicCall.extract_byte_little_endian(pAssumptionHolder.varVal)
		#return NonDeterministicCall(value)
		
		if isFromMap2Check:
			#assum_l,assum_r= assumption.split(' == ')
			#if assum_l.find('\\')== 0: assum_l = assum_l.replace('\\','',1)
			try:
				_,val = pAssumptionHolder.assumption.split(' == ')
			except Exception as ex:
				if IS_DEBUG:
					print(TColors.FAIL+ ' Error in File (fromAssumptionHolder,isFromMap2Check):'+ TColors.ENDC, benchmark)
				val = '0'
		else:
			#assum_l,assum_r= assumption.split('=')
			#assum_r = assum_r.replace(';','',1)
			#if assum_r[-1] == "f" or assum_r[-1] == "l": 
			#	assum_r = assum_r[:-1]

		#TODO: handle f and l
		#_, right = pAssumptionHolder.assumption.split("=")
		#assum_l = assum_l.strip()
		#assum_r = assum_r.strip()	

			#TODO: copy this
			try:
				_,val = pAssumptionHolder.assumption.split("=")
				val,_ = val.split(';')
				if val[-1] == "f" or val[-1] == "l":
					val = val[:-1]
			except Exception as ex:
				if IS_DEBUG:
					print(TColors.FAIL+ ' Error in File (fromAssumptionHolder):'+ TColors.ENDC, benchmark)
				val = '0'
				#return None
				pass

		value = NonDeterministicCall.extract_byte_little_endian(val.strip())
		nonDeterministicCall = NonDeterministicCall(value)
		lineNumberForNonDetCallsLst_tmp = []
		if lineNumberForNonDetCallsLst is not None:
			for (line,funcName) in lineNumberForNonDetCallsLst:
				if pAssumptionHolder.line == line:
					lineNumberForNonDetCallsLst_tmp.append((line,funcName))
			lineNumberForNonDetCallsLst_tmp_len = len(lineNumberForNonDetCallsLst_tmp)
			if lineNumberForNonDetCallsLst_tmp_len == 1:
				(_,funcName) = lineNumberForNonDetCallsLst_tmp [0]
				nonDeterministicCall.inputType = InputType.GetInputTypeForNonDetFunc(funcName)
			#else:
				#TODO: hanle if many nonDetFuncs in the same line; Now as unkown
			
		return nonDeterministicCall

	def debugInfo(self):
		print("Nondet call: {0}".format(self.value))


class SourceCodeChecker(object):
	"""
		This class will read the original source file and checks if lines from assumptions contains nondeterministic calls	
	"""

	__lines__ = None

	def __init__(self, source, plstAssumptionHolder,pIsFromMap2Check = False):
		"""
		Default constructor.

		Parameters
		----------
		source : str
			Path to source code file (absolute/relative)
		plstAssumptionHolder : [AssumptionHolder]
			List containing all lstAssumptionHolder of the witness
		"""
		assert(os.path.isfile(source))
		assert(plstAssumptionHolder is not None)
		self.source = source
		self.lstAssumptionHolder = plstAssumptionHolder
		#self.__lines__ = None
		self.isFromMap2Check = pIsFromMap2Check

	@staticmethod
	def loadSourceFromFile(pFile):
		SourceCodeChecker.__lines__ = open(pFile, "r").readlines()

	def __openfile__(self):
		"""Open file in READ mode"""
		SourceCodeChecker.__lines__ = open(self.source, "r").readlines()

	def __is_not_repeated__(self, i):
		if self.isFromMap2Check : return True
		
		x_AssumptionHolder = self.lstAssumptionHolder[i]
		y_AssumptionHolder = self.lstAssumptionHolder[i+1]

		if x_AssumptionHolder.line != y_AssumptionHolder.line:
			return True

		try:
			_, x_right = x_AssumptionHolder.assumption.split("=")
			_, y_right = y_AssumptionHolder.assumption.split("=")
			return x_right != y_right
		except Exception as ex:
			if IS_DEBUG:
				print(TColors.FAIL+ 'Error in File (__is_not_repeated__):'+TColors.ENDC , benchmark)
		
		return True	

	def __isNonDet__(self, p_AssumptionHolder):
		global lineNumberForNonDetCallsLst
		#global lineNumberForNonDetCallsLst_Map2check
		"""
			Checks if p_AssumptionHolder is nondet by checking if line contains __VERIFIER_nondet
			
		"""

		if self.isFromMap2Check : return True
		if p_AssumptionHolder is None: return False
		if "=" in p_AssumptionHolder.assumption:
			check_cast = p_AssumptionHolder.assumption.split("=")
			if len(check_cast) > 1:
				if check_cast[1].startswith(" ( struct "):
					return False
		
		if self.isFromMap2Check == False and lineNumberForNonDetCallsLst is not None:
			for (line,_) in lineNumberForNonDetCallsLst:
				if p_AssumptionHolder.line == line: return True
		if self.isFromMap2Check and lineNumberForNonDetCallsLst is not None:
			for (line,_) in lineNumberForNonDetCallsLst:
				if p_AssumptionHolder.line == line: return True

		if lineNumberForNonDetCallsLst is not None: return False
		#print('We must not hier')
		#exit(0)
		if SourceCodeChecker.__lines__ is None:
			self.__openfile__()
		lineContent = ''
		try:
			lineContent = SourceCodeChecker.__lines__[p_AssumptionHolder.line - 1]
		except:
			pass
		# At first we do not care about variable name or nondet type
		# TODO: Add support to variable name
		# TODO: Add support to nondet type
		
		#print('LiNE', lineContent)
		#index = lineContent.find('__VERIFIER_nondet_')
		#if index >= 0:
		#	lineContent=lineContent.replace(';', ',')
			#lineLst =['  unsigned int i', ' n=__VERIFIER_nondet_uint()', ' sn=0', '\n']
			#lineLst ['  unsigned long pat_len = __VERIFIER_nondet_ulong()', ' a_len = __VERIFIER_nondet_ulong()', '\n']

			#lineLst=lineContent.split(',')
			#print('lineLst', lineLst)
			#for stmt in lineLst:
			#	if stmt.find('__VERIFIER_nondet_') >=0 :
			#		left,right = stmt.split('=');
			#		left = left.strip()
			#		left_lst = left.split(' ')
			#		left=left_lst[:-1]
			#		print('LLLLLEEEFT',left)
			#		return p_AssumptionHolder.varName == left				
					

		result = lineContent.split("__VERIFIER_nondet_")
		return len(result) > 1
		#return False
		# return right != ""

	"""
	return list of NonDeterministicCall objects.
	"""
	def getNonDetAssumptions(self):
		filtered_assumptions = list()
		#print('self.lstAssumptionHolder',self.lstAssumptionHolder)
		#if len(self.lstAssumptionHolder)==0 :
		#	return []
		
		for i in range(len(self.lstAssumptionHolder)-1):
			if self.__is_not_repeated__(i):
				filtered_assumptions.append(self.lstAssumptionHolder[i])
		if len(self.lstAssumptionHolder)>0:
			filtered_assumptions.append(self.lstAssumptionHolder[-1])
		return [NonDeterministicCall.fromAssumptionHolder(x,self.isFromMap2Check) for x in filtered_assumptions if not x is None and self.__isNonDet__(x)]

	def debugInfo(self):
		for x in self.getNonDetAssumptions():
			x.debugInfo()


class TestCompMetadataGenerator(object):
	def __init__(self, metadata):
		"""
		Default constructor.

		Parameters
		----------
		metadata : { key: value}
			A dictionary containing metada info
		"""
		self.metadata = metadata

	def writeMetadataFile(self):
		""" Write metadata.xml file """
		root = ET.Element("test-metadata")
		# TODO: add support to enter function
		ET.SubElement(root, 'entryfunction').text = 'main'
		ET.SubElement(root, 'specification').text = property_file_content.strip()
		properties = {'sourcecodelang', 'sourcecodelang', 'producer',
					  'programfile', 'programhash', 'architecture', 'creationtime'}
		for property in properties:
			# 16.05.2020 && 29.05.2020
			if (category_property == Property.cover_branches or category_property == Property.cover_error_call):
				if property == 'programfile':
					ET.SubElement(root, property).text= benchmark
				elif property == 'programhash':
					ET.SubElement(root, property).text= GetSH1ForFile(benchmark)
				else:
					ET.SubElement(root, property).text = self.metadata[property]

			else:
				ET.SubElement(root, property).text = self.metadata[property]
		
		ET.ElementTree(root).write(META_DATA_FILE)
		with open(META_DATA_FILE, 'r') as original: data = original.read()
		with open(META_DATA_FILE, 'w') as modified: modified.write('<?xml version="1.0" encoding="UTF-8" standalone="no"?><!DOCTYPE test-metadata PUBLIC "+//IDN sosy-lab.org//DTD test-format test-metadata 1.0//EN" "https://sosy-lab.org/test-format/test-metadata-1.0.dtd">' + data)

class TestCompGenerator(object):
	def __init__(self, nondetList):
		"""
		Default constructor.

		Parameters
		----------
		value : [NonDeterministicCall]
			All NonDeterministicCalls from the program
		"""
		self.__root__ = ET.Element("testcase")
		for inputData in nondetList:
			if not inputData is None:
				ET.SubElement(self.__root__, "input").text = inputData.value

	def writeTestCase(self, outputFileName):
		"""
		Write testcase into XML file.

		Parameters
		----------
		outputFileName : str
			filename (with extension)
		"""
		with open(outputFileName,'w') as fTestcaseObj:
			fTestcaseObj.write('<?xml version="1.0" encoding="UTF-8" standalone="no"?><!DOCTYPE testcase PUBLIC "+//IDN sosy-lab.org//DTD test-format testcase 1.0//EN" "https://sosy-lab.org/test-format/testcase-1.0.dtd">') 
			ET.ElementTree(self.__root__).write(fTestcaseObj,'unicode')

def __getNonDetAssumptions__(witness, source, isFromMap2Check = False):
	
	"""
	return list of NonDeterministicCall objects.
	"""
	assumptionParser = AssumptionParser(witness,isFromMap2Check)
	assumptionParser.parse()
	assumptions = assumptionParser.assumptions
	return SourceCodeChecker(source, assumptions,isFromMap2Check).getNonDetAssumptions()

# error-call
def createTestFile(witness, source,testCaseFileName,isFromMap2Check = False):
	global hasInputInTestcase
	global map2CheckInputCount
	global singleValueFromMap2Check
	global lastInputInTestcaseCount
	#global mustRunTwice
	#global runNumber
	if not os.path.isfile(witness) : return
	assumptions = __getNonDetAssumptions__(witness, source,isFromMap2Check)
	#print('WE HAVE', len(assumptions))
	assumptionsLen = len(assumptions)
	if(assumptionsLen > 0):
		TestCompGenerator(assumptions).writeTestCase(testCaseFileName)
		AddFileToArchive(testCaseFileName,TEST_SUITE_DIR_ZIP)
		hasInputInTestcase=True
		if isFromMap2Check :
			if assumptionsLen > map2CheckInputCount:
				map2CheckInputCount = assumptionsLen
			if assumptionsLen == 1:
				try:
					singleValueFromMap2CheckTmp = int(assumptions[0].value) * 4
					if singleValueFromMap2CheckTmp != 0: singleValueFromMap2Check = singleValueFromMap2CheckTmp
				except:
					pass
		
		if assumptionsLen > lastInputInTestcaseCount:
			lastInputInTestcaseCount = assumptionsLen

	#metadataParser = MetadataParser(witness)
	#metadataParser.parse()
	#TestCompMetadataGenerator(metadataParser.metadata).writeMetadataFile()
	#if mustRunTwice == False or (mustRunTwice and runNumber==1):
	#	AddFileToArchive(META_DATA_FILE,TEST_SUITE_DIR_ZIP)


class Result:
	success = 1
	fail_deref = 2
	fail_memtrack = 3
	fail_free = 4
	fail_reach = 5
	fail_overflow = 6
	err_timeout = 7
	err_memout = 8
	err_unwinding_assertion = 9
	force_fp_mode = 10
	unknown = 11
	fail_memcleanup = 12
	fail_termination = 13
	#20.05.2020
	fail_cover_error_call = 14
	fail_cover_branches = 15
	

	@staticmethod
	def is_fail(res):
		if res == Result.fail_deref:
			return True
		if res == Result.fail_free:
			return True
		if res == Result.fail_memtrack:
			return True
		if res == Result.fail_overflow:
			return True
		if res == Result.fail_reach:
			return True
		if res == Result.fail_memcleanup:
			return True
		if res == result.fail_termination:
			return True
		return False

	@staticmethod
	def is_out(res):
		if res == Result.err_memout:
			return True
		if res == Result.err_timeout:
			return True
		if res == Result.unknown:
			return True
		return False


class Property:
	unreach_call = 1
	memsafety = 2
	overflow = 3
	termination = 4
	memcleanup = 5
	cover_branches = 6
	cover_error_call = 7 # 20.05.2020
#29.05.2020
def CompileFile(fil, include_dir = '.'):
	fil=os.path.abspath(fil)
	if not os.path.isfile(fil):
		print('FILE:',fil, 'not exists')
		exit(0)
	cmd=[C_COMPILER,'-c',fil , '-o', INSTRUMENT_OUTPUT_FILE_OBJ,'-I'+include_dir]
	p=subprocess.Popen(cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE)
	while True:
		line = p.stdout.readline()
		if not line: break
		line_de=line.decode('utf-8')
		print(line_de)
	while True:
		line = p.stderr.readline()
		if not line: break
		line_de=line.decode('utf-8')
		print(line_de)
	if not os.path.isfile(INSTRUMENT_OUTPUT_FILE_OBJ):
		print('Cannot compile: ',fil)
		exit(0)

def RunCPAChecker():
	global witness_file_name
	print("\nValidating ......\n" )
	print('witness_file_name:',witness_file_name)
	cpa_cmd = [CPA_CHECKER_EXE,'-witnessValidation' ,'-setprop', 'witness.checkProgramHash=false' ,
			'-heap', '5000m', '-benchmark', '-setprop', 'cpa.predicate.memoryAllocationsAlwaysSucceed=true',
			'-setprop', 'cpa.smg.memoryAllocationFunctions=malloc,__kmalloc,kmalloc,kzalloc,kzalloc_node,ldv_zalloc,ldv_malloc',
			'-setprop', 'cpa.smg.arrayAllocationFunctions=calloc,kmalloc_array,kcalloc', '-setprop', 
			'cpa.smg.zeroingMemoryAllocation=calloc,kzalloc,kcalloc,kzalloc_node,ldv_zalloc', '-setprop',
			'cpa.smg.deallocationFunctions=free,kfree,kfree_const', 
			'-witness', os.path.abspath(witness_file_name),
			'-timelimit', '90s', '-stats', '-spec', os.path.abspath(property_file),
			'-' + str(arch) ,
			os.path.abspath(benchmark)]
	print (TColors.BOLD,'Command: ', ' '.join(cpa_cmd), TColors.ENDC)
	p=subprocess.Popen(cpa_cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE , cwd = INSTRUMENT_OUTPUT_DIR)
	while True:
		line = p.stderr.readline()
		if not line: break
		line_de=line.rstrip().decode('utf-8')
		print(line_de)	

	while True:
		line = p.stdout.readline()
		if not line: break
		line_de=line.rstrip().decode('utf-8')
		print(line_de)

def RunCovTest():
	print("\nValidating Test-Cases ...\n")
	global toWorkSourceFile
	cov_test_exe_abs=os.path.abspath(COV_TEST_EXE)
	cov_test_cmd =[cov_test_exe_abs]
	cov_test_cmd.extend(COV_TEST_PARAMS)
	test_suite_dir_zip_abs=os.path.abspath(TEST_SUITE_DIR_ZIP)
	property_file_abs = os.path.abspath(property_file)
	#if category_property == Property.cover_error_call:
	#	benchmark_abs = os.path.abspath(toWorkSourceFile)
	#else:
	benchmark_abs = os.path.abspath(benchmark)
	sourceForTestCov = WRAPPER_OUTPUT_DIR + '/' + os.path.basename(benchmark_abs)
	sourceForTestCov = os.path.abspath(sourceForTestCov)
	testCovOutputDir = os.path.abspath(INSTRUMENT_OUTPUT_DIR + '/output_cov')
	cov_test_cmd.extend(['--output', testCovOutputDir])
	print('sourceForTestCov',sourceForTestCov)
	run_without_output(' '.join(['cp',benchmark_abs ,sourceForTestCov]))
	cov_test_cmd.extend(['-'+str(arch),'--test-suite' ,test_suite_dir_zip_abs , '--goal' ,property_file_abs , sourceForTestCov])
	print(' '.join(cov_test_cmd))
	#cwd = INSTRUMENT_OUTPUT_DIR
	p=subprocess.Popen(cov_test_cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE , cwd = INSTRUMENT_OUTPUT_DIR)
	while True:
		line = p.stderr.readline()
		if not line: break
		line_de=line.rstrip().decode('utf-8')
		print(line_de)	
		
	while True:
		line = p.stdout.readline()
		if not line: break
		line_de=line.rstrip().decode('utf-8')
		print(line_de)

def run_without_output(cmd_line, pCwd = None):
	if(SHOW_ME_OUTPUT): print(cmd_line)
	the_args = shlex.split(cmd_line)
	try:
		if not pCwd is None:
			p = subprocess.run(the_args, stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,cwd=pCwd)
		else:
			p = subprocess.run(the_args, stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
	except Exception as ex:
		print(ex)
	#p.communicate()

# Function to run esbmc
def run(cmd_line):
	global important_outs_by_ESBMC
	
	if(SHOW_ME_OUTPUT): print (TColors.BOLD,'Command: ', cmd_line, TColors.ENDC)
	outs=['' for i in range(MAX_NUM_OF_LINES_OUT)]
	errs=['' for i in range(MAX_NUM_OF_LINES_ERRS)]
	important_outs=[]
	important_errs=[]
	index=-1;
	index_err=-1
	the_args = shlex.split(cmd_line)
	p = None
	try:
		p = subprocess.Popen(the_args,stdout=subprocess.PIPE,stderr=subprocess.PIPE) #bufsize=1
		#print('pid',p.pid)
		nbsr_out = NonBlockingStreamReader(p.stdout)
		nbsr_err = NonBlockingStreamReader(p.stderr)
		
		while nbsr_out.hasMore():
			IsTimeOut(True)
			try:
				output = nbsr_out.readline(1) # second =0.01
				# 0.1 secs to let the shell output the result
				if output:
					index =(index + 1) % MAX_NUM_OF_LINES_OUT
					if(SHOW_ME_OUTPUT): print(output)
					isAddedToImportant=False
					for out_by_ESBMC in important_outs_by_ESBMC:
						if out_by_ESBMC in output:
							important_outs.append(output)
							isAddedToImportant=True
							break
					if not isAddedToImportant : outs[index]= output
				else:
					#IsTimeOut(True)
					#time.sleep(0.1)
					pass
			except UnexpectedEndOfStream as ueos:
				pass
		
		while nbsr_err.hasMore():
			IsTimeOut(True)
			try:
				err = nbsr_err.readline(1)
				# 0.1 secs to let the shell output the result
				if err:
					index_err =(index_err + 1) % MAX_NUM_OF_LINES_ERRS
					if(SHOW_ME_OUTPUT): print(err)
					isAddedToImportant=False
					for out_by_ESBMC in important_outs_by_ESBMC:
						if out_by_ESBMC in err:
							important_errs.append(err)
							isAddedToImportant=True
							break
					if not isAddedToImportant : errs[index_err]= err
				else:
					#IsTimeOut(True)
					#time.sleep(0.1)
					pass
			except UnexpectedEndOfStream as ueos:
				pass
		#(stdout, stderr) = p.communicate()
		#print (stdout.decode(), stderr.decode())
		#return stdout.decode()
	except MyTimeOutException as e:
		if p is not None:
			try:
				p.kill()
			except Exception:
				pass
		raise e
		pass
	except KeyboardInterrupt:
		global is_ctrl_c
		is_ctrl_c = True
		#exit(0)
		pass
	#getTime(p.pid)
	# Kill ESBMC When Timeout (maybe)
	if p is not None:
		try:
			p.kill()
		except Exception:
			pass
	out_str=''
	for imp in important_errs:
		out_str += imp
	#Part 1
	for loop in range(index_err,MAX_NUM_OF_LINES_ERRS):
		out_str += errs[loop]
	#part 02
	for loop in range(0,index_err+1):
		out_str += errs[loop]
	for imp in important_outs:
		out_str += imp
	#Part 1
	for loop in range(index,MAX_NUM_OF_LINES_OUT):
		out_str += outs[loop]
	#part 02
	for loop in range(0,index+1):
		out_str += outs[loop]
	return out_str

def runWithTimeoutEnabled(cmd_line,pCwd=None):
	if(SHOW_ME_OUTPUT): print (TColors.BOLD, 'Command: ', cmd_line ,TColors.ENDC)
	the_args = shlex.split(cmd_line)
	p = None
	try:
		p = subprocess.Popen(the_args,stdout=subprocess.PIPE,stderr=subprocess.PIPE,cwd=pCwd,shell=False) #bufsize=1
		nbsr_out = NonBlockingStreamReader(p.stdout)
		nbsr_err = NonBlockingStreamReader(p.stderr)
		
		while nbsr_out.hasMore():
			IsTimeOut(True)
			try:
				output = nbsr_out.readline(1) # second 0.01
				# 0.1 secs to let the shell output the result
				if output:
					if(SHOW_ME_OUTPUT): print(output)
				else:
					IsTimeOut(True)
					#time.sleep(0.1)
			except UnexpectedEndOfStream as ueos:
				pass
		
		while nbsr_err.hasMore():
			IsTimeOut(True)
			try:
				err = nbsr_err.readline(1) # 0.01
				# 0.1 secs to let the shell output the result
				if err:
					if(SHOW_ME_OUTPUT): print(err)
				else:
					IsTimeOut(True)
					#time.sleep(0.1)
			except UnexpectedEndOfStream as ueos:
				pass

	#except MyTimeOutException as e:
	#	pass
	except KeyboardInterrupt:
		global is_ctrl_c
		is_ctrl_c = True
		#print('CTRLLLLLLLLLL')
		#exit(0)
		pass
	# Kill ESBMC When Timeout (maybe)
	if p is not None:
		try:
			if p.poll() is None: # proc still working
				#os.killpg(os.getpgid(p.pid), signal.SIGTERM)
				#os.killpg(os.getpgid(p.pid), signal.SIGKILL)
				p.kill()
				p.terminate()
				print('Killed......')
		except Exception as ex:
			#print('EXXXXXXXX')
			#print(ex)
			pass

def parse_result(the_output, prop):	
	# Parse output
	#if prop == Property.cover_error_call or prop == Property.cover_branches:
	#	raise Exception("Don't use parse_result for cover_error_call, cover_branches.!!")
	if "Timed out" in the_output:
		return Result.err_timeout
	if "Out of memory" in the_output:
		return Result.err_memout
	
	if "Chosen solver doesn\'t support floating-point numbers" in the_output:
		return Result.force_fp_mode

	# Error messages:
	memory_leak = "dereference failure: forgotten memory"
	invalid_pointer = "dereference failure: invalid pointer"
	access_out = "dereference failure: Access to object out of bounds"
	dereference_null = "dereference failure: NULL pointer"
	expired_variable = "dereference failure: accessed expired variable pointer"
	invalid_object = "dereference failure: invalidated dynamic object"
	invalid_object_free = "dereference failure: invalidated dynamic object freed"
	invalid_pointer_free = "dereference failure: invalid pointer freed"
	free_error = "dereference failure: free() of non-dynamic memory"
	bounds_violated = "array bounds violated"
	free_offset = "Operand of free must have zero pointer offset"
	if "VERIFICATION FAILED" in the_output:
		if "unwinding assertion loop" in the_output:
			return Result.err_unwinding_assertion
		
		if prop == Property.memcleanup:
			if memory_leak in the_output:
				return Result.fail_memcleanup
		if prop == Property.termination:
			return Result.fail_termination
		if prop == Property.memsafety:
			if memory_leak in the_output:
				return Result.fail_memtrack
			if invalid_pointer_free in the_output:
				return Result.fail_free
			if invalid_object_free in the_output:
				return Result.fail_free
			if expired_variable in the_output:
				return Result.fail_deref
			if invalid_pointer in the_output:
				return Result.fail_deref
			if dereference_null in the_output:
				return Result.fail_deref
			if free_error in the_output:
				return Result.fail_free
			if access_out in the_output:
				return Result.fail_deref
			if invalid_object in the_output:
				return Result.fail_deref
			if bounds_violated in the_output:
				return Result.fail_deref
			if free_offset in the_output:
				return Result.fail_free
			if " Verifier error called" in the_output:
				return Result.success
		if prop == Property.overflow:
			return Result.fail_overflow
		if prop == Property.unreach_call:
			return Result.fail_reach
	if "VERIFICATION SUCCESSFUL" in the_output:
		return Result.success
	return Result.unknown

def get_result_string(the_result):
	# TODO: What is the output
	if the_result == Result.fail_cover_error_call: return "FAIL_COVER_ERROR_CALL"	
	if the_result == Result.fail_cover_branches: return "FAIL_COVER_BRANCHES"
	if the_result == Result.fail_memcleanup: return "FALSE_MEMCLEANUP"
	if the_result == Result.fail_memtrack: return "Unknown" #return "FALSE_MEMTRACK"
	if the_result == Result.fail_free: return "Unknown" #return "FALSE_FREE"
	if the_result == Result.fail_deref: return "Unknown" #return "FALSE_DEREF"
	if the_result == Result.fail_overflow: return "FALSE_OVERFLOW"
	if the_result == Result.fail_reach: return "FALSE_REACH"
	if the_result == Result.fail_termination: return "FALSE_TERMINATION"
	if the_result == Result.success: return "TRUE" #return "DONE"
	if the_result == Result.err_timeout: return "Timed out"
	if the_result == Result.err_unwinding_assertion: return "Unknown"
	if the_result == Result.err_memout: return "Unknown"
	if the_result == Result.unknown: return "Unknown"
	exit(0)
'''
def get_result_string(the_result):
	if the_result == Result.fail_memcleanup: return "FALSE_MEMCLEANUP"
	if the_result == Result.fail_memtrack: return "FALSE_MEMTRACK"
	if the_result == Result.fail_free: return "FALSE_FREE"
	if the_result == Result.fail_deref: return "FALSE_DEREF"
	if the_result == Result.fail_overflow: return "FALSE_OVERFLOW"
	if the_result == Result.fail_reach: return "DONE"
	if the_result == Result.success: return "DONE"
	if the_result == Result.err_timeout: return "Timed out"
	if the_result == Result.err_unwinding_assertion: return "Unknown"
	
	# TODO: What is the output
	if the_result == Result.fail_cover_error_call: return "FAIL_COVER_ERROR_CALL"	
	if the_result == Result.fail_cover_branches: return "FAIL_COVER_BRANCHES"
	if the_result == Result.err_memout: return "Unknown"
	if the_result == Result.unknown: return "TIMEOUT"
	exit(0)
'''


def get_command_line(strat, prop, arch, benchmark, fp_mode):
	global goals_count
	global mustRunTwice
	global runNumber
	global mustRunTwice_MemOverflowReachTerm
	command_line = esbmc_path + esbmc_dargs
	command_line += benchmark + " --quiet "
	if arch == 32:
		command_line += "--32 "
	else:
		command_line += "--64 "
	# Add witness arg , Now Added in Verify method.
	#witness_file_name = os.path.basename(benchmark) + ".graphml "
	#if prop != Property.cover_branches and prop != Property.cover_error_call:
	#	command_line += "--witness-output " + witness_file_name
	#BEGIN
	# Special case for termination, it runs regardless of the strategy
	if prop == Property.termination:
		if mustRunTwice_MemOverflowReachTerm and runNumber ==1:
			command_line += "--partial-loops --no-pointer-check --no-bounds-check --no-assertions --termination --max-inductive-step 3 "
		else:
			command_line += "--no-pointer-check --no-bounds-check --no-assertions --termination --max-inductive-step 3 "
		return command_line	
	
	if prop == Property.overflow:
		if mustRunTwice_MemOverflowReachTerm and runNumber ==1:
			command_line += "--partial-loops --no-pointer-check --no-bounds-check --overflow-check --no-assertions "
		else:
			command_line += "--no-pointer-check --no-bounds-check --overflow-check --no-assertions "
	elif prop == Property.memsafety:
		strat = "incr"
		if mustRunTwice_MemOverflowReachTerm and runNumber ==1:
			command_line += "--partial-loops --memory-leak-check --no-assertions "
		else:
			command_line += "--memory-leak-check --no-assertions "
	elif prop == Property.memcleanup:
		strat = "incr"
		if mustRunTwice_MemOverflowReachTerm and runNumber ==1:
			command_line += "--partial-loops --memory-leak-check --no-assertions "
		else:
			command_line += "--memory-leak-check --no-assertions "
	elif prop == Property.unreach_call:
		if mustRunTwice_MemOverflowReachTerm and runNumber ==1:
			command_line += "--partial-loops --no-pointer-check --no-bounds-check --interval-analysis "
		else:
			command_line += "--no-pointer-check --no-bounds-check --interval-analysis "
	elif prop == Property.cover_branches:		
		if(goals_count>100):
			if (goals_count<250):
				command_line += "--max-k-step 10 --unwind 1 --no-pointer-check --no-bounds-check --interval-analysis --no-slice "
			else:
				command_line += "--max-k-step 10 --unwind 1 --no-pointer-check --no-bounds-check --interval-analysis --no-slice "
		elif (goals_count<100):
			command_line += "--unlimited-k-steps --unwind 1 --no-pointer-check --no-bounds-check --interval-analysis --no-slice "
		#20.05.2020 + #03.06.2020 kaled: adding the option "--unlimited-k-steps" for coverage_error_call .... --max-k-step 5
	elif prop == Property.cover_error_call:
	#kaled : 03.06.2020 --unwind 10 --partial-loops
		if mustRunTwice and runNumber ==1:
			command_line += "--partial-loops --no-pointer-check --no-bounds-check --interval-analysis --no-slice "
		else:
			command_line += "--unlimited-k-steps --no-pointer-check --no-bounds-check --interval-analysis --no-slice "
	else:
		print ("Unknown property")
		exit(1)
	
	# Add strategy
	if strat == "fixed":
		command_line += "--k-induction --max-inductive-step 3 "
	elif strat == "kinduction":
		#TODO: Check this
		#command_line += "--bidirectional "
		command_line += "--k-induction --max-inductive-step 3 "
	elif strat == "falsi":
		command_line += "--falsification "
	elif strat == "incr":
		command_line += "--incremental-bmc "
	else:
		print ("Unknown strategy")
		exit(1)
	#END
	# if we're running in FP mode, use MathSAT
	if fp_mode:
		command_line += "--mathsat "
	return command_line

# not more used
def generate_metadata_from_witness(p_witness_file):	
	global META_DATA_FILE
	global TEST_SUITE_DIR_ZIP
	
	if not os.path.isfile(p_witness_file): return
	metadataParser = MetadataParser(p_witness_file)
	metadataParser.parse()
	if len(metadataParser.metadata) > 0 :
		TestCompMetadataGenerator(metadataParser.metadata).writeMetadataFile()
		AddFileToArchive(META_DATA_FILE , TEST_SUITE_DIR_ZIP)
#is str a float 
def isfloat(str):
	try:
		a = float(str)
	except (TypeError, ValueError): return False
	else: return True
#is str an Int
def isint(str):
	try:
		a = float(str) ; b = int(str)
	except (TypeError, ValueError): return False
	else: return a == b

def generate_testcase_from_assumption(p_test_case_file_full,p_inst_assumptions):
	global FuSeBMCFuzzerLib_COVERBRANCHES_MAX_TESTCASE_SIZE_BYTES, FuSeBMCFuzzerLib_COVERBRANCHES_NUM_OF_GENERATED_TESTCASES
	#if(arch == 32)if(arch == 32)
	# sys.byteorder 'little' or 'big'
	inst_len = len(p_inst_assumptions)
	if (inst_len > 0 ) : print('    Generate Testcase ...')
	seed_f = None
	testcaseSize = 0
	if FuSeBMCFuzzerLib_COVERBRANCHES_ENABLED == True and FuSeBMCFuzzerLib_COVERBRANCHES_DONE == False:
		print('    Generate Seed ...')
		#if(inst_len > FuSeBMCFuzzerLib_COVERBRANCHES_MAX_TESTCASE_SIZE_BYTES): FuSeBMCFuzzerLib_COVERBRANCHES_MAX_TESTCASE_SIZE_BYTES = inst_len
		if (inst_len > 0) : FuSeBMCFuzzerLib_COVERBRANCHES_NUM_OF_GENERATED_TESTCASES += 1
		
		seed_filename = os.path.join(FuSeBMCFuzzerLib_COVERBRANCHES_SEED_DIR,os.path.basename(p_test_case_file_full) + '.bin')
		seed_f = open(seed_filename, 'wb')
	
	with open(p_test_case_file_full, 'w') as testcase_file:
		testcase_file.write('<?xml version="1.0" encoding="UTF-8" standalone="no"?>')
		testcase_file.write('<!DOCTYPE testcase PUBLIC "+//IDN sosy-lab.org//DTD test-format testcase 1.0//EN" "https://sosy-lab.org/test-format/testcase-1.0.dtd">')
		testcase_file.write('<testcase>')
		for nonDeterministicCall in p_inst_assumptions:
			# if you want to print to std
			#print(nonDeterministicCall)
			cType =''
			if nonDeterministicCall.inputType != InputType.unkonwn:
				cType = ' type="'+InputType.GetAs_C_DataType(nonDeterministicCall.inputType)+'"'
			testcase_file.write('<input'+cType+'>'+nonDeterministicCall.value +'</input>')
			if seed_f is not None:
				is_signed = not InputType.IsUnSigned(nonDeterministicCall.inputType)
				if nonDeterministicCall.inputType == InputType._string:
					bytesNum = len(nonDeterministicCall.value)
				else:
					bytesNum = InputType.GeSizeForDataType(nonDeterministicCall.inputType, arch)
				testcaseSize += bytesNum
				if(isint(nonDeterministicCall.value)):
					seed_f.write((int(nonDeterministicCall.value)).to_bytes(bytesNum, byteorder=sys.byteorder, signed=is_signed))
				elif (isfloat(nonDeterministicCall.value)):
					seed_f.write((float(nonDeterministicCall.value)).to_bytes(bytesNum, byteorder=sys.byteorder, signed=is_signed))
				else:
					input_str = nonDeterministicCall.value + ''
					if input_str and not isinstance(input_str , bytes): input_str = input_str.encode()
					seed_f.write(input_str)
					#seed_f.write((nonDeterministicCall.value + '').encode())
		testcase_file.write('</testcase>')
		if seed_f is not None:
			if(testcaseSize > 0 and testcaseSize > FuSeBMCFuzzerLib_COVERBRANCHES_MAX_TESTCASE_SIZE_BYTES): FuSeBMCFuzzerLib_COVERBRANCHES_MAX_TESTCASE_SIZE_BYTES = testcaseSize
			seed_f.close()
	
	AddFileToArchive(p_test_case_file_full , TEST_SUITE_DIR_ZIP)
	
def getLinesCountInFile(pFilePath):
	line_count = 0
	file = open(pFilePath, 'r')
	for line in file:
		if line != '\n' : line_count += 1
	file.close()
	return 	line_count
#replace GOAL_1: with fuseGoalCalled(1);
def ReplaceGoalLabelWithFuseGoalCalledMethod(src_file, dest_file):
	goalRegex = re.compile('GOAL_([0-9]+):')
	with open(dest_file,'w') as fout:
		if FuSeBMCFuzzerLib_COVERBRANCHES_ENABLED:
			fout.write('unsigned int fuSeBMC_category = 2 ;\n')
			#fout.write('const char * fuSeBMC_run_id = "'+fuSeBMC_run_id+'";\n')
			fout.write('extern void fuSeBMC_return(int code);\n')
		fout.write('extern void fuseGoalCalled(unsigned long int goal);\n')
		with open(src_file,'r') as fin:
			for line in fin:
				regex_result = goalRegex.match(line)
				if regex_result is not None:
					#print(regex_result.string,regex_result.group(1))
					line = line.replace(regex_result.string,'fuseGoalCalled('+regex_result.group(1)+');\n')
				fout.write(line)
# return lstFuSeBMC_FuzzerGoals
def RunAFLForCoverBranches(instrumented_afl_file):
	global FuSeBMCFuzzerLib_COVERBRANCHES_COVERED_GOALS_FILE , FuSeBMCFuzzerLib_COVERBRANCHES_DONE
	afl_min_length = FuSeBMCFuzzerLib_COVERBRANCHES_MAX_TESTCASE_SIZE_BYTES
	l_seed_dir = FuSeBMCFuzzerLib_COVERBRANCHES_SEED_DIR
	if afl_min_length <= 0 :
		afl_min_length = FuSeBMCFuzzerLib_COVERBRANCHES_DEFAULT_AFL_MIN_LENGTH
		l_seed_dir = os.path.abspath(AFL_HOME_PATH+ '/seeds/s1')
	#else:
	#	run_without_output(' '.join(['cp','./seeds_extra/xxxxxx.bin' ,l_seed_dir]))
	print('\n\nRun FuSeBMCFuzzer ....\n')
	#afl_min_length *= 3
	if IS_DEBUG: print('afl_min_length=', afl_min_length , ' Byte(s)')
	try:
		FuSeBMCFuzzerLib_COVERBRANCHES_DONE = True
		#ReplaceGoalLabelWithFuseGoalCalledMethod(INSTRUMENT_OUTPUT_FILE, instrumented_afl_file)
		afl_fuzzer_bin = os.path.abspath(WRAPPER_OUTPUT_DIR+'/instrumented_afl.exe')
		lstFuSeBMC_FuzzerGoals = []
		os.environ["AFL_QUIET"] = "1"
		os.environ["AFL_DONT_OPTIMIZE"] = "1"
		runWithTimeoutEnabled(' '.join([AFL_HOME_PATH + '/afl-gcc','-D exit=fuSeBMC_exit', '-D abort=fuSeBMC_abort_prog',
								'-D ___assert_fail=fuSeBMC___assert_fail',
								'-m'+str(arch), instrumented_afl_file, '-o',afl_fuzzer_bin,
								'-L./FuSeBMC_FuzzerLib/','-lFuSeBMC_FuzzerLib_'+str(arch),'-lm','-lpthread']))
		if not os.path.isfile(afl_fuzzer_bin): raise Exception (afl_fuzzer_bin + ' Not found.')
		#os.environ["fusebmc_env"] = "houssam"
		os.environ["AFL_SKIP_CPUFREQ"] = "1"
		os.environ["AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES"] = "1"
		os.environ["AFL_SKIP_CRASHES"] = "1"
		os.environ["FUZZ_UNTIL_ERROR"] = "1"
		
		#runWithTimeoutEnabled(' '.join(['timeout','-k','2s', str(FuSeBMCFuzzerLib_COVERBRANCHES_TIMEOUT), AFL_HOME_PATH + '/afl-fuzz',
		#						'-i', AFL_HOME_PATH +'/seeds/rsafety-in','-o' ,'./aflOutputDir', '-m', '1200',
		#						'-x', AFL_HOME_PATH +'/dictionaries/selToken.dict','--', afl_fuzzer_bin]),WRAPPER_OUTPUT_DIR)
		#'-x', AFL_HOME_PATH +'/seeds/rsafety-in'
		runWithTimeoutEnabled(' '.join(['timeout' ,'-s', 'SIGKILL', '-k','2s', str(FuSeBMCFuzzerLib_COVERBRANCHES_TIMEOUT),
			 AFL_HOME_PATH + '/afl-fuzz','-n',
			'-l',str(afl_min_length),
			'-i', l_seed_dir,'-o' ,'./aflOutputDir',
			'-m', '1200','-t','120000+','--', afl_fuzzer_bin]),WRAPPER_OUTPUT_DIR)
	
		if os.path.isfile(FuSeBMCFuzzerLib_COVERBRANCHES_COVERED_GOALS_FILE):
			with open(FuSeBMCFuzzerLib_COVERBRANCHES_COVERED_GOALS_FILE,'r') as f_covered:
				for line in f_covered:
					line_int = int(line)
					lstFuSeBMC_FuzzerGoals.append(line_int)
					#lstGoalsToWorkOn.remove(line_int)

		#print('fuSeBMC_run_id:', os.environ[fuSeBMC_run_id])
		#os.unsetenv(fuSeBMC_run_id)
		#sys.exit(0)
		
		print('    lstFuSeBMC_FuzzerGoals',lstFuSeBMC_FuzzerGoals, '\n')
		return lstFuSeBMC_FuzzerGoals
	except MyTimeOutException as mytime_ex: raise mytime_ex
	except KeyboardInterrupt as kb_ex: raise kb_ex;
	except Exception as ex: 
		print(TColors.FAIL); print('Exception',ex); print(TColors.ENDC)
		return None

def verify(strat, prop, fp_mode):
	global is_ctrl_c
	global remaining_time_s
	global hasInputInTestcase
	global lastInputInTestcaseCount 
	global goals_count
	global mustRunTwice
	global runNumber
	global __testSuiteDir__
	global singleValueFromMap2Check
	global lineNumberForNonDetCallsLst
	global witness_file_name
	
	#sglobal MUST_APPLY_TIME_PER_GOAL
	lastInputInTestcaseCount = 5 # default
	singleValueFromMap2Check = 0
	goal_id=0
	goal_witness_file_full=''
	inst_assumptions=[]
	if(prop == Property.cover_branches):
		try:
			paramAddElse = '--add-else'
			paramAddLabelAfterLoop = '--add-label-after-loop'
			paramAddGoalAtEndOfFunc='--add-goal-at-end-of-func'
			paramExportGoalInfo = ''
			
			if MUST_APPLY_LIGHT_INSTRUMENT_FOR_BIG_FILES:
				linesCountInSource = getLinesCountInFile(benchmark)
				if linesCountInSource >= BIG_FILE_LINES_COUNT:
					paramAddElse = ''
					paramAddLabelAfterLoop=''
					paramAddGoalAtEndOfFunc = ''

			addFuSeBMCFuncParam = ''
			paramHandleReturnInMain = ''
			if FuSeBMCFuzzerLib_COVERBRANCHES_ENABLED : paramHandleReturnInMain = '--handle-return-in-main'
			if MUST_RUN_MAP_2_CHECK_FOR_BRANCHES: addFuSeBMCFuncParam = '--add-FuSeBMC-func'
			if goalSorting == GoalSorting.DEPTH_THEN_TYPE or goalSorting == GoalSorting.TYPE_THEN_DEPTH:
				paramExportGoalInfo = '--export-goal-info'
			infoFile = WRAPPER_OUTPUT_DIR + '/info.xml'

			runWithTimeoutEnabled(' '.join([FUSEBMC_INSTRUMENT_EXE_PATH, '--input',benchmark ,'--output', INSTRUMENT_OUTPUT_FILE , 
								'--goal-output-file',INSTRUMENT_OUTPUT_GOALS_FILE, paramAddElse,'--add-labels', '--export-line-number-for-NonDetCalls', paramExportGoalInfo,
								paramAddLabelAfterLoop, paramAddGoalAtEndOfFunc, addFuSeBMCFuncParam, paramHandleReturnInMain,
								'--info-file', infoFile,
								'--compiler-args','-I'+os.path.dirname(os.path.abspath(benchmark))]))
			
			#Without else + without label-after-loop but with goal at end of function
			#run_without_output(' '.join([FUSEBMC_INSTRUMENT_EXE_PATH, '--input',benchmark ,'--output', INSTRUMENT_OUTPUT_FILE , 
			#					  '--goal-output-file',INSTRUMENT_OUTPUT_GOALS_FILE,'--add-labels','--add-goal-at-end-of-func',
			#					  '--compiler-args', '-I'+os.path.dirname(os.path.abspath(benchmark))]))
			if os.path.isfile(INSTRUMENT_OUTPUT_GOALS_FILE):
				goals_count_file = open(INSTRUMENT_OUTPUT_GOALS_FILE, "r")
				goals_count = int(goals_count_file.read())
				goals_count_file.close()
			lstGoalsToWorkOn , lstFuSeBMC_FuzzerGoals, lineNumberForNonDetCallsLst = [] , [] , []
			isInfoFileParsingOK = False
			try:
				rootXML = ET.parse(infoFile).getroot()
				if os.path.isfile(infoFile):
					elemLineNumberForNonDetCalls= rootXML.find('nonDetCalls')
					if elemLineNumberForNonDetCalls is not None and elemLineNumberForNonDetCalls.text != 'ERROR':
						for nonDetCall in elemLineNumberForNonDetCalls.iter('nonDetCall'):
							elemNonDetCall_line = nonDetCall.find('line')
							elemNonDetCall_funcName = nonDetCall.find('funcName')
							if elemNonDetCall_line is not None and elemNonDetCall_funcName is not None:
								lineNumberForNonDetCallsLst.append((int(elemNonDetCall_line.text),elemNonDetCall_funcName.text))
					else:
						lineNumberForNonDetCallsLst = None
			except Exception as ex:
				if IS_DEBUG: print(TColors.FAIL, ex, TColors.ENDC)
			if (goalSorting == GoalSorting.DEPTH_THEN_TYPE or goalSorting == GoalSorting.TYPE_THEN_DEPTH) and os.path.isfile(infoFile):
				ifGoalsLst, compoundGoalsLst, loopGoalsLst, elseGoalsLst, \
										endOfFuncGoalsLst, afterLoopGoalsLst, emptyElseGoalsLst, \
										forGoalsLst, cXXForRangeGoalsLst, doWhileGoalsLst, whileGoalsLst = \
										[],[],[],[],[],[],[],[],[],[],[]
				goalInfoDict = dict()
				try:
					#elemGoalsCount = rootXML.find('goalsCount')
					#if elemGoalsCount is not None and elemGoalsCount.text: goalsCC = int(elemGoalsCount.text)
					elemIf = rootXML.find('if')
					if elemIf is not None and elemIf.text: ifGoalsLst = [int(x) for x in elemIf.text.split(',') if x]
					elemLoop = rootXML.find('loop')
					if elemLoop is not None and elemLoop.text: loopGoalsLst = [int(x) for x in elemLoop.text.split(',') if x]
					elemAfterLoop = rootXML.find('afterLoop')
					if elemAfterLoop is not None and elemAfterLoop.text: afterLoopGoalsLst = [int(x) for x in elemAfterLoop.text.split(',') if x]
					elemElse= rootXML.find('else')
					if elemElse is not None and elemElse.text: elseGoalsLst = [int(x) for x in elemElse.text.split(',') if x]
					elemEmptyElse= rootXML.find('emptyElse')
					if elemEmptyElse is not None and elemEmptyElse.text: emptyElseGoalsLst = [int(x) for x in elemEmptyElse.text.split(',') if x]
					elemCompound= rootXML.find('compound')
					if elemCompound is not None and elemCompound.text: compoundGoalsLst = [int(x) for x in elemCompound.text.split(',') if x]
					elemEndOfFunc= rootXML.find('endOfFunc')
					if elemEndOfFunc is not None and elemEndOfFunc.text: endOfFuncGoalsLst = [int(x) for x in elemEndOfFunc.text.split(',') if x]
					elemFor= rootXML.find('For')
					if elemFor is not None and elemFor.text: forGoalsLst = [int(x) for x in elemFor.text.split(',') if x]
					elemCXXForRange= rootXML.find('CXXForRange')
					if elemCXXForRange is not None and elemCXXForRange.text: cXXForRangeGoalsLst = [int(x) for x in elemCXXForRange.text.split(',') if x]
					elemDoWhile= rootXML.find('DoWhile')
					if elemDoWhile is not None and elemDoWhile.text: doWhileGoalsLst = [int(x) for x in elemDoWhile.text.split(',') if x]
					elemWhile= rootXML.find('While')
					if elemWhile is not None and elemWhile.text: whileGoalsLst = [int(a) for a in elemWhile.text.split(',') if a]
					
					elemGoalInfos= rootXML.find('goalInfos')
					if elemGoalInfos is not None and elemGoalInfos.text != 'ERROR':
						for goalInfo in elemGoalInfos.iter('goalInfo'):
							elemGoalInfo_goal = goalInfo.find('goal')
							elemGoalInfo_depth = goalInfo.find('depth')
							if elemGoalInfo_goal is not None and elemGoalInfo_depth is not None:
								goalInfoDict[int(elemGoalInfo_goal.text)] = int(elemGoalInfo_depth.text)
					del rootXML
					### Sort the Goals					
					for g in ifGoalsLst: lstGoalsToWorkOn.append(GoalInfo(g, GoalType.IF, goalInfoDict.get(g, -1))) # default = -1
					for g in elseGoalsLst: lstGoalsToWorkOn.append(GoalInfo(g, GoalType.ELSE, goalInfoDict.get(g, -1)))
					for g in forGoalsLst: lstGoalsToWorkOn.append(GoalInfo(g, GoalType.FOR, goalInfoDict.get(g, -1)))
					for g in cXXForRangeGoalsLst: lstGoalsToWorkOn.append(GoalInfo(g, GoalType.CXX_FOR_RANGE, goalInfoDict.get(g, -1)))
					for g in doWhileGoalsLst: lstGoalsToWorkOn.append(GoalInfo(g, GoalType.DO_WHILE, goalInfoDict.get(g, -1)))
					for g in whileGoalsLst: lstGoalsToWorkOn.append(GoalInfo(g, GoalType.WHILE, goalInfoDict.get(g, -1)))
					for g in compoundGoalsLst: lstGoalsToWorkOn.append(GoalInfo(g, GoalType.COMPOUND, goalInfoDict.get(g, -1)))
					for g in loopGoalsLst: lstGoalsToWorkOn.append(GoalInfo(g, GoalType.LOOP, goalInfoDict.get(g, -1)))
					for g in endOfFuncGoalsLst: lstGoalsToWorkOn.append(GoalInfo(g, GoalType.END_OF_FUNCTION, goalInfoDict.get(g, -1)))
					for g in afterLoopGoalsLst: lstGoalsToWorkOn.append(GoalInfo(g, GoalType.AFTER_LOOP, goalInfoDict.get(g, -1)))
					for g in emptyElseGoalsLst: lstGoalsToWorkOn.append(GoalInfo(g, GoalType.EMPTY_ELSE, goalInfoDict.get(g, -1)))
					lst_len = len(lstGoalsToWorkOn)
					# BubbleSort
					if goalSorting == GoalSorting.DEPTH_THEN_TYPE:
						for i in range(0, lst_len):
							for j in range(0, lst_len-i-1):
								if ((lstGoalsToWorkOn[j].depth < lstGoalsToWorkOn[j + 1].depth) or \
									((lstGoalsToWorkOn[j].depth == lstGoalsToWorkOn[j + 1].depth) and (lstGoalsToWorkOn[j].goalType > lstGoalsToWorkOn[j + 1].goalType))):
										temp = lstGoalsToWorkOn[j]
										lstGoalsToWorkOn[j]= lstGoalsToWorkOn[j + 1]
										lstGoalsToWorkOn[j + 1]= temp
						# Applay IF at First & EmptyElse at End
						if goalSorting_IF_first or goalSorting_EMPTYELSE_last:
							tmp_if_lst, tmp_emptyElse_lst, tmp_afterLoop_lst, rest_lst = [], [], [] , []
							for ginfo in lstGoalsToWorkOn:
								if goalSorting_IF_first and ginfo.goalType == GoalType.IF: tmp_if_lst.append(ginfo)
								elif goalSorting_AFTERLOOP_last and ginfo.goalType == GoalType.AFTER_LOOP: tmp_afterLoop_lst.append(ginfo)
								elif goalSorting_EMPTYELSE_last and ginfo.goalType == GoalType.EMPTY_ELSE: tmp_emptyElse_lst.append(ginfo)
								else : rest_lst.append(ginfo)
							lstGoalsToWorkOn = tmp_if_lst + rest_lst + tmp_afterLoop_lst + tmp_emptyElse_lst
							del tmp_if_lst, tmp_emptyElse_lst, tmp_afterLoop_lst, rest_lst
							
					else: #GoalSorting.TYPE_THEN_DEPTH:
						for i in range(0, lst_len):
							for j in range(0, lst_len-i-1):
								if (lstGoalsToWorkOn[j].goalType > lstGoalsToWorkOn[j + 1].goalType or \
									(lstGoalsToWorkOn[j].goalType == lstGoalsToWorkOn[j + 1].goalType and lstGoalsToWorkOn[j].depth < lstGoalsToWorkOn[j + 1].depth)):
										temp = lstGoalsToWorkOn[j]
										lstGoalsToWorkOn[j]= lstGoalsToWorkOn[j + 1]
										lstGoalsToWorkOn[j + 1]= temp

					if IS_DEBUG : assert (len(lstGoalsToWorkOn) == goals_count)
					isInfoFileParsingOK = True
					if SHOW_ME_OUTPUT:
						print('ifGoalsLst',ifGoalsLst)
						print('compoundGoalsLst',compoundGoalsLst)
						print('loopGoalsLst',loopGoalsLst)
						print('elseGoalsLst',elseGoalsLst)
						print('endOfFuncGoalsLst',endOfFuncGoalsLst)
						print('afterLoopGoalsLst',afterLoopGoalsLst)
						print('emptyElseGoalsLst',emptyElseGoalsLst)
						print('forGoalsLst',forGoalsLst)
						print('cXXForRangeGoalsLst',cXXForRangeGoalsLst)
						print('doWhileGoalsLst',doWhileGoalsLst)
						print('whileGoalsLst',whileGoalsLst)
						print('goalInfoDict','(goal : depth)',goalInfoDict)
				except Exception as ex:
					if IS_DEBUG:
						print(TColors.FAIL, ex, TColors.ENDC)
						#traceback.print_exc()
				del ifGoalsLst, compoundGoalsLst, loopGoalsLst, elseGoalsLst, endOfFuncGoalsLst, \
				afterLoopGoalsLst, emptyElseGoalsLst, \
										forGoalsLst, cXXForRangeGoalsLst, doWhileGoalsLst, whileGoalsLst
				if not isInfoFileParsingOK:
					lstGoalsToWorkOn = [GoalInfo(g, GoalType.NONE , -1) for g in range(1,goals_count+1)]
					
			else: #GoalSorting.SEQUENTIAL
				lstGoalsToWorkOn = [GoalInfo(g, GoalType.NONE , -1) for g in range(1,goals_count+1)]
			if IS_DEBUG:
				print('lstGoalsToWorkOn:')
				for ginfo in lstGoalsToWorkOn: print(ginfo.toString())
				#exit(0)
			IsTimeOut(True)
			#check if FuSeBMC_inustrument worked
			if not os.path.isfile(INSTRUMENT_OUTPUT_FILE):
				print("Cannot instrument the file.")
				if IS_DEBUG:
					print(TColors.FAIL,'Cannot instrument the file.',TColors.ENDC)
					exit(0)
				#return Result.unknown
			if not os.path.isfile(INSTRUMENT_OUTPUT_GOALS_FILE):
				print("Cannot instrument the file, goalFile cannot be found.")
				if IS_DEBUG:
					print(TColors.FAIL,'Cannot instrument the file, goalFile cannot be found.',TColors.ENDC)
					exit(0)
				#return Result.unknown

			if MUST_COMPILE_INSTRUMENTED:
				CompileFile(INSTRUMENT_OUTPUT_FILE,os.path.dirname(os.path.abspath(benchmark)))
			
			if FuSeBMCFuzzerLib_COVERBRANCHES_ENABLED:
				instrumentedAFL_src = WRAPPER_OUTPUT_DIR + '/instrumented_afl.c'
				ReplaceGoalLabelWithFuseGoalCalledMethod(INSTRUMENT_OUTPUT_FILE, instrumentedAFL_src)
			goalTracerExecOK = False
			if FuSeBMC_GoalTracerLib_Enabled:
				instrumentedTracer = WRAPPER_OUTPUT_DIR + '/instrumented_tracer.c'
				instrumentedTracerExec = os.path.abspath(WRAPPER_OUTPUT_DIR + '/instrumented_tracer.exe')
				goals_covered_file = WRAPPER_OUTPUT_DIR + '/goals_covered.txt'
				wrapperHomeDir = os.path.dirname(__file__)
				gccArch = '-m' + str(arch)
				if not FuSeBMCFuzzerLib_COVERBRANCHES_ENABLED:
					ReplaceGoalLabelWithFuseGoalCalledMethod(INSTRUMENT_OUTPUT_FILE, instrumentedTracer)
				else:
					instrumentedTracer = instrumentedAFL_src # the same file from fuzzer.
				# add file hier 
				runWithTimeoutEnabled(' '.join(['gcc',gccArch, instrumentedTracer,'-o',instrumentedTracerExec, 
											'-L'+wrapperHomeDir+'/FuSeBMC_GoalTracerLib/','-lFuSeBMC_GoalTracerLib_'+str(arch),'-lm','-lpthread']))
				goalTracerExecOK = os.path.isfile(instrumentedTracerExec)

			goals_count_original = goals_count
			goals_covered = 0
			goals_covered_lst = []
			goals_covered_by_map2check=[]
			lstFuSeBMC_GoalTracerGoals=[]
			goals_to_be_run_map2check = []
			goalInTheMiddle = 0
			if goals_count > 0: goalInTheMiddle = int(len(lstGoalsToWorkOn) / 2)
			#if MUST_APPLY_TIME_PER_GOAL and goals_count>0 :
			#	time_per_goal_for_esbmc=int(time_out_s) / goals_count
			#	time_per_goal_for_esbmc =int(time_per_goal_for_esbmc) # ms to second
			#	if time_per_goal_for_esbmc == 0 : time_per_goal_for_esbmc = 1
			
			#list of list of NonDeterministicCall: each NonDeterministicCall has a value
			#inst_all_assumptions=[]

			#counter=0
			singleValueFromMap2Check = 0
			isFromMap2Check=False
			SourceCodeChecker.loadSourceFromFile(INSTRUMENT_OUTPUT_FILE)
			linesInSource = len(SourceCodeChecker.__lines__)
			if IS_DEBUG:
				print(TColors.OKGREEN,'Lines In source:',linesInSource,TColors.ENDC)
			if MUST_RUN_MAP_2_CHECK_FOR_BRANCHES:
				lstGoalsToWorkOnLen = len(lstGoalsToWorkOn)
				if(lstGoalsToWorkOnLen > 2): goals_to_be_run_map2check.append(lstGoalsToWorkOn[1].goal)
				if(lstGoalsToWorkOnLen > 5): goals_to_be_run_map2check.append(lstGoalsToWorkOn[4].goal)
				goals_to_be_run_map2check.append(lstGoalsToWorkOn[goalInTheMiddle].goal)
				goals_to_be_run_map2check.append(lstGoalsToWorkOn[-1].goal)
				#if goals_count > 100:
				#	goals_to_be_run_map2check.extend([33,44,55]) # for example
				#if goals_count > 100:
				#	goals_to_be_run_map2check.extend(range(80,90)) # for example
				
			print('\nRunning FuSeBMC for Cover-Branches:\n')
			if FuSeBMCFuzzerLib_COVERBRANCHES_ENABLED:
				instrumentedESBMC = WRAPPER_OUTPUT_DIR + '/instrumented_esbmc.c'
				with open(instrumentedESBMC,'w') as fout:
					fout.write('extern void fuSeBMC_return(int code){}\n')
					with open(INSTRUMENT_OUTPUT_FILE,'r') as fin:
						for line in fin: fout.write(line)
				if lineNumberForNonDetCallsLst is not None:
					lineNumberForNonDetCallsLst = [(line_nr + 1,funcName) for (line_nr,funcName) in lineNumberForNonDetCallsLst] # icrease by 1 Line.
			else: instrumentedESBMC = INSTRUMENT_OUTPUT_FILE
			if SHOW_ME_OUTPUT: print('lineNumberForNonDetCallsLst','(line,funcName)',lineNumberForNonDetCallsLst)
			lstFuSeBMC_FuzzerGoals = []
			## Starting Goals LOOP !!!
			#for goalInfo in lstGoalsToWorkOn:
			counter = 0
			if MUST_APPLY_TIME_PER_GOAL : time_for_goal_max = int(time_out_s / 5)
			while len(lstGoalsToWorkOn) > 0:
				counter += 1
				goalInfo = lstGoalsToWorkOn.pop(0)
				i = goalInfo.goal
				isFromMap2Check = False
				goal_id = i
				IsTimeOut(True)
				if FuSeBMCFuzzerLib_COVERBRANCHES_ENABLED and not FuSeBMCFuzzerLib_COVERBRANCHES_DONE \
					and (FuSeBMCFuzzerLib_COVERBRANCHES_NUM_OF_GENERATED_TESTCASES >= FuSeBMCFuzzerLib_COVERBRANCHES_NUM_OF_GENERATED_TESTCASES_TO_RUN_AFL \
					or remaining_time_s < time_out_s / 3 ):
					lstFuSeBMC_FuzzerGoals = RunAFLForCoverBranches(instrumentedAFL_src)
				
				param_timeout_esbmc = ''
				time_for_goal = -1
				param_memlimit_esbmc = ''
				if MUST_APPLY_TIME_PER_GOAL:
					factor=2 # 1/2 of the remaining time
					if counter < goals_count / 2 : factor=3 # 1/3 of the remaining time
					#remaining_time_s = int(remaining_time_ms / 1000) # ms to second
					if counter < goals_count and remaining_time_s > 10:
						time_for_goal = int(remaining_time_s/factor)
						if counter < goals_count / 2 and time_for_goal > time_for_goal_max :
							time_for_goal = time_for_goal_max
						if time_for_goal < 20 : time_for_goal = -1
					if time_for_goal > 0 : param_timeout_esbmc = ' --timeout ' + str(time_for_goal) + 's '
				if MEM_LIMIT_BRANCHES_ESBMC > 0:
					param_memlimit_esbmc = ' --memlimit ' + str(MEM_LIMIT_BRANCHES_ESBMC) + 'g '
					
				inst_assumptions=[]
				goal='GOAL_'+str(i)
				
				if(SHOW_ME_OUTPUT): print(TColors.OKGREEN+'+++++++++++++++++++++++++++++++'+TColors.ENDC)
				print('------------------------------------')
				print('STARTING : ' + goalInfo.toString())
				
				# You can use or True to run all
				if MUST_RUN_MAP_2_CHECK_FOR_BRANCHES and (goal_id in goals_to_be_run_map2check):
					isFromMap2Check = True
					test_case_file_full=os.path.join(__testSuiteDir__,'testcase_'+str(i)+'_map.xml')
					goal_witness_file_full = map2checkWitnessFile
					RemoveFileIfExists(map2checkWitnessFile)
					sedOutputPath = WRAPPER_OUTPUT_DIR+'/fusebmc_instrument_output/sed_' + goal + '.c'
					sed_cmd_line = ' '.join(['sed',"'s/"+goal+':'+"/FuSeBMC_custom_func()/g'", INSTRUMENT_OUTPUT_FILE])
					try:
						sedOutFile = open(sedOutputPath, 'a')
						if FuSeBMCFuzzerLib_COVERBRANCHES_ENABLED :
							sedOutFile.write('void fuSeBMC_return(int code){}\n')
							sedOutFile.flush()						
						pSed = subprocess.run(shlex.split(sed_cmd_line), stdout=sedOutFile,stderr=subprocess.DEVNULL)
						sedOutFile.flush()
						sedOutFile.close()
						#map2checkTimeForBranches : can be caculated !!
						#map2CheckNonDetGenerator = 'symex' if linesInSource >= 11000 else 'fuzzer'
						map2CheckNonDetGenerator = 'fuzzer'
						runWithTimeoutEnabled(' '.join(['timeout',str(map2checkTimeForBranches)+'s', map2check_path,'--timeout',str(map2checkTimeForBranches),'--fuzzer-mb', str(MEM_LIMIT_BRANCHES_MAP2CHECK),'--nondet-generator',map2CheckNonDetGenerator , '--target-function','--target-function-name', 'FuSeBMC_custom_func', '--generate-witness',os.path.abspath(sedOutputPath)]), WRAPPER_OUTPUT_DIR)
						if os.path.isfile(map2checkWitnessFile):
							#createTestFile(map2checkWitnessFile,sedOutputPath, testCaseFileName ,True)
							inst_assumptions=__getNonDetAssumptions__(map2checkWitnessFile,INSTRUMENT_OUTPUT_FILE,True)
							inst_assumptions_len = len(inst_assumptions)
							run_without_output(' '.join(['cp',map2checkWitnessFile,WRAPPER_OUTPUT_DIR + '/map2check_'+str(goal_id)+'.graphml']))
							if inst_assumptions_len > 0 :
								if inst_assumptions_len == 1:
									try:
										singleValueFromMap2CheckTmp = int(inst_assumptions[0].value) * 4
										if singleValueFromMap2CheckTmp != 0: singleValueFromMap2Check = singleValueFromMap2CheckTmp
									except:
										pass
								generate_testcase_from_assumption(test_case_file_full,inst_assumptions)
								if inst_assumptions_len > lastInputInTestcaseCount: lastInputInTestcaseCount = inst_assumptions_len
								hasInputInTestcase = True
								goals_covered_lst.append(i)
								goals_covered_by_map2check.append(i)
								
						# comment or uncomment kaled ...
						#continue: means don't execute ESBMC on goal_id
						#if len(inst_assumptions)>0: # for example
						#	continue
						#if goals_count > 20: # for example
						#	continue
						#continue # always # you can not use it again
					except MyTimeOutException as mytime_ex: raise mytime_ex
					except KeyboardInterrupt as kb_ex: raise kb_ex;
					except Exception as ex: print(TColors.FAIL); print(ex); print(TColors.ENDC)
					
					# End of MUST_RUN_MAP_2_CHECK_FOR_BRANCHES
				if FuSeBMC_GoalTracerLib_Enabled and goalTracerExecOK and goal_id in lstFuSeBMC_GoalTracerGoals:
					print('    is already covered by other goals.')
					continue
				if FuSeBMCFuzzerLib_COVERBRANCHES_ENABLED and FuSeBMCFuzzerLib_COVERBRANCHES_DONE and goal_id in lstFuSeBMC_FuzzerGoals:
					print('    is already covered by FuSeBMCFuzzerLib...')
					continue
				print('    time_for_goal:' , time_for_goal , 's')
				#if (goals_count > 1000 and goal_id % 2 == 0): continue # for example
				isFromMap2Check = False
				goal_witness_file=goal+'.graphml'
				goal_witness_file_full=os.path.join(INSTRUMENT_OUTPUT_DIR ,goal_witness_file)
				test_case_file_full=os.path.join(__testSuiteDir__,'testcase_'+str(i)+'_ES.xml')
				inst_esbmc_command_line = get_command_line(strat, prop, arch, instrumentedESBMC, fp_mode)
				inst_new_esbmc_command_line = inst_esbmc_command_line + ' --witness-output ' + goal_witness_file_full + ' --error-label ' + goal \
												+ ' -I'+os.path.dirname(os.path.abspath(benchmark)) + param_timeout_esbmc + param_memlimit_esbmc
												# + ' --timeout ' + str(time_per_goal_for_esbmc)+ 's '
				#print('COMMAAND:'+inst_new_esbmc_command_line)
				output = run(inst_new_esbmc_command_line)
				IsTimeOut(True)
				if not os.path.isfile(goal_witness_file_full):
					print('Cannot run ESBMC for '+ goal)
				else:
					#if not IsMetaDataGenerated:
					#	generate_metadata_from_witness(goal_witness_file_full)
					
					# it is only for __VERIFIER_nondet_int but not __VERIFIER_nondet_uint
					inst_assumptions=__getNonDetAssumptions__(goal_witness_file_full,INSTRUMENT_OUTPUT_FILE)
					
					#inst_all_assumptions.append(inst_assumptions)
					if len(inst_assumptions)>0 :
						if len(inst_assumptions) > lastInputInTestcaseCount: lastInputInTestcaseCount = len(inst_assumptions)
						hasInputInTestcase=True
						goals_covered += 1
						goals_covered_lst.append(i)
						#22.06.2020
						generate_testcase_from_assumption(test_case_file_full,inst_assumptions)
						if FuSeBMC_GoalTracerLib_Enabled and goalTracerExecOK:
							RemoveFileIfExists(goals_covered_file)
							proc_inst = None
							try:
								input_lst = [nonDeterministicCall.value for nonDeterministicCall in inst_assumptions]
								input_str = '\n'.join([inp for inp in input_lst])
								process = subprocess.Popen(['timeout' ,'5s',instrumentedTracerExec],
														stdin=subprocess.PIPE if input_str else None,
														stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=False, cwd=WRAPPER_OUTPUT_DIR )
								if input_str and not isinstance(input_str, bytes):
									input_str = input_str.encode()
								output, err_output = process.communicate(input=input_str
																		#, timeout=timelimit if timelimit else None
									)
								returncode = process.poll()
								
								#proc_inst = subprocess.Popen(['timeout' ,'5s',instrumentedTracerExec],stdin=subprocess.PIPE, stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL, cwd=WRAPPER_OUTPUT_DIR)
								#for nonDeterministicCall in inst_assumptions:
								#	input_str = nonDeterministicCall.value + '\n'
								#if input_str and not isinstance(input_str, bytes):
								#	input_str = input_str.encode()
								#proc_inst.stdin.write(input_str)
								#proc_inst.stdin.flush()
								#proc_inst.wait()
								#print('proc_inst.returncode=',proc_inst.returncode)
								
								if os.path.isfile(goals_covered_file):
									line_int = 0
									lstGoalsInFile=[]
									with open(goals_covered_file,'r') as f_covered:
										for line in f_covered:
											line_int = int(line)
											lstGoalsInFile.append(line_int)
											if line_int not in lstFuSeBMC_GoalTracerGoals:
												lstFuSeBMC_GoalTracerGoals.append(line_int)
									print('   ', goal,'covers:',lstGoalsInFile)
									#lstGoalsToWorkOn = [ginf for ginf in lstGoalsToWorkOn if not ginf.goal in lstGoalsInFile]
									if IS_DEBUG:
										goals_covered_file_for_goal = WRAPPER_OUTPUT_DIR + '/goals_covered_'+str(goal_id)+'.txt'
										run_without_output(' '.join(['mv', goals_covered_file, goals_covered_file_for_goal]))															
							except Exception as ex:
								if proc_inst is not None: proc_inst.kill()
								if IS_DEBUG:
									print('ERROR:',ex)
									exit(0)
					else:
						#goals_to_be_covered_with_extra_lst.append(i)
						pass
				

			#here we write many testcases;we can write one
			#for one_list in inst_all_assumptions:
			#	counter+=1
		except MyTimeOutException as e:
			#print('Timeout !!!')
			pass
		except KeyboardInterrupt:
			#print('CTRL + C')
			pass
		#IsTimeOut(False)
		#print('remaining_time_s=',remaining_time_s)
		#if not IsMetaDataGenerated:
		#	generate_metadata_from_witness(goal_witness_file_full)
		
		if os.path.isfile(goal_witness_file_full) and not os.path.isfile(test_case_file_full):
			inst_assumptions=__getNonDetAssumptions__(goal_witness_file_full,INSTRUMENT_OUTPUT_FILE, isFromMap2Check)
			#inst_all_assumptions.append(inst_assumptions)
			if len(inst_assumptions)>0 :
				if len(inst_assumptions) > lastInputInTestcaseCount: lastInputInTestcaseCount = len(inst_assumptions)
				hasInputInTestcase=True
				
				if isFromMap2Check : 
					goals_covered_by_map2check.append(goal_id)
				else:
					goals_covered += 1
					goals_covered_lst.append(goal_id)
				#22.06.2020
				generate_testcase_from_assumption(test_case_file_full,inst_assumptions) 
			else:
				pass

		if not is_ctrl_c and FuSeBMCFuzzerLib_COVERBRANCHES_ENABLED and not FuSeBMCFuzzerLib_COVERBRANCHES_DONE:
			lstFuSeBMC_FuzzerGoals = RunAFLForCoverBranches(instrumentedAFL_src)
		
		if MUST_GENERATE_RANDOM_TESTCASE or MUST_ADD_EXTRA_TESTCASE:
			extra_test_case_id = goals_count + 1
		
		#hasInputInTestcase=False # for test
		if MUST_GENERATE_RANDOM_TESTCASE: #and not hasInputInTestcase:
			random_testcase_file=os.path.join(__testSuiteDir__,'testcase_'+str(extra_test_case_id)+'_Fu.xml')
			extra_test_case_id += 1
			randomMaxRange = 5 # hh
			rndLst=[]
			if lastInputInTestcaseCount > 0:
				randomMaxRange = lastInputInTestcaseCount + 1
			if randomMaxRange == 3:
				randomMaxRange -= 1
				rndLst=[NonDeterministicCall('0')] + \
						[NonDeterministicCall(str(randrange(-128,128))) for i in range(1,randomMaxRange)]
			elif randomMaxRange == 2:
				rndLst = [NonDeterministicCall(str(singleValueFromMap2Check))] # singleValueFromMap2Check may be Zero ?
			else:
				randomMaxRange -= 2
				rndLst = [NonDeterministicCall('0')] + \
							[NonDeterministicCall(str(randrange(-128,128))) for i in range(1,randomMaxRange)]+[NonDeterministicCall('0')]
			#randomMaxRange = 36
			#rndLst = [NonDeterministicCall('0')]
			#		[NonDeterministicCall('0')] + \
			#		[NonDeterministicCall('0')] # note : Two Zeros
			generate_testcase_from_assumption(random_testcase_file, rndLst)# was 5
		
		if MUST_ADD_EXTRA_TESTCASE:
			lst2=[NonDeterministicCall('0'),NonDeterministicCall('128')] * int(lastInputInTestcaseCount/2)
			if lastInputInTestcaseCount % 2 == 1:
				lst2.append(NonDeterministicCall('0'))
			lst4=[]
			for i in range(0,lastInputInTestcaseCount):
				if i % 3 == 0 : lst4.append(NonDeterministicCall('0'))
				if i % 3 == 1 : lst4.append(NonDeterministicCall('128'))
				if i % 3 == 2 : lst4.append(NonDeterministicCall('-256'))
			
			#randomMaxRange = 36
			if singleValueFromMap2Check == 0: singleValueFromMap2Check = 128 # No Sigle Value; default 128
			elif singleValueFromMap2Check < 0: singleValueFromMap2Check *= -1
			
			if IS_DEBUG: print(TColors.OKGREEN,'singleValueFromMap2Check=',singleValueFromMap2Check,TColors.ENDC)
			lst5 = [NonDeterministicCall(str(randrange(-singleValueFromMap2Check,singleValueFromMap2Check))) for i in range(1,lastInputInTestcaseCount)] + \
					[NonDeterministicCall('0'), NonDeterministicCall('0')]
			lst6 = [NonDeterministicCall('0')]
				
			extra_lsts=[[NonDeterministicCall(str(randrange(-128,128))) for _ in range(0,lastInputInTestcaseCount-1)],
						lst2,
						[NonDeterministicCall('128')]+[NonDeterministicCall('0') for _ in range(0,lastInputInTestcaseCount-1)],
						lst4, lst5 , lst6]
			for l in extra_lsts:
				extra_testcase_file=os.path.join(__testSuiteDir__,'testcase_'+str(extra_test_case_id)+'_Fu.xml')
				extra_test_case_id += 1
				generate_testcase_from_assumption(extra_testcase_file,l)

		#ZipDir(__testSuiteDir__ ,TEST_SUITE_DIR_ZIP)
		if RUN_COV_TEST:
			RunCovTest() 
		if IS_DEBUG:
			print('fuSeBMC_run_id:',fuSeBMC_run_id)
			if MUST_ADD_EXTRA_TESTCASE:
				print('goals_count',goals_count)
			print('goals_count_original',goals_count_original)
			print('goals_covered',goals_covered)
			print('goals_covered_lst',goals_covered_lst)
			if MUST_RUN_MAP_2_CHECK_FOR_BRANCHES:
				print('goals_covered_by_map2check',goals_covered_by_map2check)
				print('goals_to_be_run_map2check', goals_to_be_run_map2check)
			if FuSeBMC_GoalTracerLib_Enabled:
				print('lstFuSeBMC_GoalTracerGoals',lstFuSeBMC_GoalTracerGoals)
			if FuSeBMCFuzzerLib_COVERBRANCHES_ENABLED:
				print('lstFuSeBMC_FuzzerGoals',lstFuSeBMC_FuzzerGoals)
			if FuSeBMC_GoalTracerLib_Enabled and FuSeBMCFuzzerLib_COVERBRANCHES_ENABLED:
				lstTracer_and_Fuzzer = [g for g in lstFuSeBMC_GoalTracerGoals if g in lstFuSeBMC_FuzzerGoals]
				print('lstTracer_and_Fuzzer',lstTracer_and_Fuzzer)
				
				lstTracer_but_not_Fuzzer = [g for g in lstFuSeBMC_GoalTracerGoals if g not in lstFuSeBMC_FuzzerGoals]
				print('lstTracer_but_not_Fuzzer',lstTracer_but_not_Fuzzer)
				
				lstFuzzer_but_not_Tracer = [g for g in lstFuSeBMC_FuzzerGoals if g not in lstFuSeBMC_GoalTracerGoals]
				print('lstFuzzer_but_not_Tracer',lstFuzzer_but_not_Tracer)
				
		
		#global start_time
		#print('Execution t_i_m_e:',time.time() - start_time,' Second.')

		# todo: what is the result
		#if(len(inst_all_assumptions)>0):
		#	return parse_result("VERIFICATION FAILED",prop)
		#else:
		#	return parse_result("VERIFICATION SUCCESSFUL",prop)
		#if IS_DEBUG:
			#global start_time
			#end_time = time.clock()
			#print(TColors.OKGREEN,'StartTime (s):', start_time, TColors.ENDC)
			#print(TColors.OKGREEN,'EndTime (s):', end_time, TColors.ENDC)
			#print(TColors.OKGREEN,'Time (s):', end_time - start_time, TColors.ENDC)
			#print(os.times())
		
		if is_ctrl_c:
			#return parse_result("something else will get unknown",prop)
			return 'DONE'
		#Important with False
		if IsTimeOut(False):
			#return parse_result("Timed out",prop)
			print('The Time is out..')
		#print('remaining_time_s=',remaining_time_s)
		#return parse_result("VERIFICATION SUCCESSFUL",prop)
		return 'DONE'
		
	if(prop == Property.cover_error_call):
		print('\nRunning FuSeBMC for Cover-Error:\n')
		if FuSeBMCFuzzerLib_ERRORCALL_ENABLED:
			afl_fuzzer_src = WRAPPER_OUTPUT_DIR+'/instrumented_afl.c'
			afl_fuzzer_bin = os.path.abspath(WRAPPER_OUTPUT_DIR+'/instrumented_afl.exe')
			fuSeBMC_Fuzzer_testcase = WRAPPER_OUTPUT_DIR + '/test-suite/FuSeBMC_Fuzzer_testcase.xml'
			try:
				runWithTimeoutEnabled(' '.join([FUSEBMC_INSTRUMENT_EXE_PATH, '--input',benchmark ,'--output', afl_fuzzer_src , 
									  '--add-func-call-in-func','fuSeBMC_reach_error=reach_error',
									  '--compiler-args', '-I'+os.path.dirname(os.path.abspath(benchmark))]))
				if not os.path.isfile(afl_fuzzer_src): raise Exception (afl_fuzzer_src + ' Not found.')
				os.system("sed -i '1i unsigned int fuSeBMC_category = 1;' " + afl_fuzzer_src) # Append text to begin of file
				os.environ["AFL_QUIET"] = "1"
				os.environ["AFL_DONT_OPTIMIZE"] = "1"
				runWithTimeoutEnabled(' '.join([AFL_HOME_PATH + '/afl-gcc',
											 '-D abort=fuSeBMC_abort_prog', '-m'+str(arch), afl_fuzzer_src, '-o',afl_fuzzer_bin,
											 '-L./FuSeBMC_FuzzerLib/','-lFuSeBMC_FuzzerLib_'+str(arch),'-lm','-lpthread']))
				if not os.path.isfile(afl_fuzzer_bin): raise Exception (afl_fuzzer_bin + ' Not found.')
				os.environ["AFL_SKIP_CPUFREQ"] = "1"
				os.environ["AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES"] = "1"
				os.environ["AFL_SKIP_CRASHES"] = "1"
				os.environ["FUZZ_UNTIL_ERROR"] = "1"
				runWithTimeoutEnabled(' '.join(['timeout', '-k','2s', str(FuSeBMCFuzzerLib_ERRORCALL_TIMEOUT), AFL_HOME_PATH + '/afl-fuzz',
											'-i', AFL_HOME_PATH +'/seeds/rsafety-in','-o' ,'./aflOutputDir', '-m', '1200',
											'-x', AFL_HOME_PATH +'/dictionaries/selToken.dict','--', afl_fuzzer_bin]),WRAPPER_OUTPUT_DIR)
				if os.path.isfile(fuSeBMC_Fuzzer_testcase): AddFileToArchive(fuSeBMC_Fuzzer_testcase,TEST_SUITE_DIR_ZIP)
			except MyTimeOutException as e: raise e
			except KeyboardInterrupt as kbe: raise kbe
			except Exception as ex :
				if IS_DEBUG : print(TColors.FAIL,' Exception', ex , TColors.ENDC)


		isFromMap2Check = False
		try:
			print('STARTING INSTRUMENTATION ... \n')
			global toWorkSourceFile
			global map2CheckInputCount
			is_test_file_created=False
			witness_file_name = ''
			testCaseFileName = None
			myLabel = 'FuSeBMC_ERROR'
			runWithTimeoutEnabled(' '.join([FUSEBMC_INSTRUMENT_EXE_PATH, '--input',benchmark ,'--output', INSTRUMENT_OUTPUT_FILE , 
									  '--add-label-in-func',myLabel + '=reach_error',
									  '--compiler-args', '-I'+os.path.dirname(os.path.abspath(benchmark))]))
			IsTimeOut(True)	 
			isInstrumentOK=True
			#check if FuSeBMC_inustrument worked
			if not os.path.isfile(INSTRUMENT_OUTPUT_FILE):
				print("Cannot instrument the file.")
				if IS_DEBUG:
					print(TColors.FAIL,'Cannot instrument the file.',TColors.ENDC)
					exit(0)
				isInstrumentOK = False
				myLabel = 'ERROR'
				toWorkSourceFile=benchmark
			else:
				toWorkSourceFile=INSTRUMENT_OUTPUT_FILE
				#return "Error"
			if MUST_COMPILE_INSTRUMENTED :
				CompileFile(toWorkSourceFile,os.path.dirname(os.path.abspath(toWorkSourceFile)))
			SourceCodeChecker.loadSourceFromFile(toWorkSourceFile)
			linesInSource = len(SourceCodeChecker.__lines__)			

			if MUST_RUN_MAP_2_CHECK_FOR_ERROR_CALL_FUZZER:
				print('STARTING GOAL 1 ... \n')
				try:
					isFromMap2Check = True
					is_test_file_created = False
					#RemoveFileIfExists(map2checkWitnessFile)
					#map2CheckNonDetGenerator = 'symex' if linesInSource >= 11000 else 'fuzzer'
					map2CheckNonDetGenerator = 'fuzzer'
					#testCaseFileName = TESTCASE_FILE_FOR_MAP_CHECK
					witness_file_name = map2checkWitnessFile
					testCaseFileName = __testSuiteDir__ + "/testcase_map_fuzzer.xml"
					runWithTimeoutEnabled(' '.join(['timeout',str(map2checkTimeErrorCall_Fuzzer)+'s', map2check_path,'--timeout',str(map2checkTimeErrorCall_Fuzzer),'--fuzzer-mb', str(MEM_LIMIT_ERROR_CALL_MAP2CHECK), '--nondet-generator', map2CheckNonDetGenerator, '--target-function', '--target-function-name','reach_error','--generate-witness',os.path.abspath(toWorkSourceFile)]), WRAPPER_OUTPUT_DIR)
					if os.path.isfile(map2checkWitnessFile):
						createTestFile(map2checkWitnessFile,toWorkSourceFile, testCaseFileName ,True)
						if MUST_RUN_MAP_2_CHECK_FOR_ERROR_CALL_SYMEX:
							run_without_output(' '.join(['cp', map2checkWitnessFile, WRAPPER_OUTPUT_DIR + '/map2check_fuzzer.graphml']))
						is_test_file_created=True
				except MyTimeOutException as e: raise e
				except KeyboardInterrupt as kbe: raise kbe
				except Exception as ex :
					if IS_DEBUG : print(TColors.FAIL,' Exception', ex , TColors.ENDC)
					#raise ex
					pass
				
			if MUST_RUN_MAP_2_CHECK_FOR_ERROR_CALL_SYMEX:
				print('STARTING GOAL 1.1 ... \n')
				try:
					isFromMap2Check = True
					is_test_file_created = False
					RemoveFileIfExists(map2checkWitnessFile)
					#map2CheckNonDetGenerator = 'symex' if linesInSource >= 11000 else 'fuzzer'
					map2CheckNonDetGenerator = 'symex'
					#testCaseFileName = TESTCASE_FILE_FOR_MAP_CHECK
					testCaseFileName = __testSuiteDir__ + "/testcase_map_symex.xml"
					witness_file_name = map2checkWitnessFile
					runWithTimeoutEnabled(' '.join(['timeout',str(map2checkTimeErrorCall_Symex)+'s', map2check_path,'--timeout',str(map2checkTimeErrorCall_Symex),'--fuzzer-mb', str(MEM_LIMIT_ERROR_CALL_MAP2CHECK),'--nondet-generator', map2CheckNonDetGenerator, '--target-function', '--target-function-name','reach_error','--generate-witness',os.path.abspath(toWorkSourceFile)]), WRAPPER_OUTPUT_DIR)
					if os.path.isfile(map2checkWitnessFile):
						createTestFile(map2checkWitnessFile,toWorkSourceFile, testCaseFileName ,True)
						#run_without_output(' '.join(['cp', map2checkWitnessFile, WRAPPER_OUTPUT_DIR + '/map2check_symex.graphml'])) # no need to copy it
						is_test_file_created=True
				except MyTimeOutException as e: raise e
				except KeyboardInterrupt as kbe: raise kbe
				except Exception as ex :
					if IS_DEBUG : print(TColors.FAIL,' Exception', ex , TColors.ENDC)
					#raise ex
					pass
			
			try:
				print('STARTING GOAL 2 ... \n')
				runNumber = 1
				is_test_file_created = False
				isFromMap2Check = False
				testCaseFileName = TESTCASE_FILE_FOR_COVER_ERROR
				witness_file_name = os.path.join(INSTRUMENT_OUTPUT_DIR,os.path.basename(benchmark) + '__1.graphml')
				esbmc_command_line = get_command_line(strat, prop, arch, toWorkSourceFile, fp_mode)			
				esbmc_command_line += ' --witness-output ' + witness_file_name +' '+'-I'+os.path.dirname(os.path.abspath(benchmark))+ ' '
				if isInstrumentOK : esbmc_command_line += ' --error-label ' + myLabel + ' '
				esbmc_command_line += ' --timeout ' + str(int(time_out_s - 1))+'s '
				esbmc_command_line += ' --memlimit ' + str(MEM_LIMIT_ERROR_CALL_ESBMC) + 'g '
				output = run(esbmc_command_line)
				IsTimeOut(True)
				res = parse_result(output, category_property)
				if(res == Result.force_fp_mode):
					fp_mode = True
					esbmc_command_line = get_command_line(strat, prop, arch, toWorkSourceFile, fp_mode)			
					esbmc_command_line += ' --witness-output ' + witness_file_name +' '+'-I'+os.path.dirname(os.path.abspath(benchmark))+ ' '
					if isInstrumentOK : esbmc_command_line += ' --error-label ' + myLabel + ' '
					esbmc_command_line += ' --timeout ' + str(int(time_out_s - 1))+'s '
					esbmc_command_line += ' --memlimit ' + str(MEM_LIMIT_ERROR_CALL_ESBMC) + 'g '
					output = run(esbmc_command_line)
					IsTimeOut(True)
					
				if os.path.isfile(witness_file_name):
					createTestFile(witness_file_name,toWorkSourceFile, testCaseFileName , False)
					is_test_file_created=True
			except MyTimeOutException as e: raise e
			except KeyboardInterrupt as kbe: raise kbe
			except Exception as ex :
				if IS_DEBUG : print(TColors.FAIL,' Exception', ex , TColors.ENDC)
			
			if mustRunTwice:
				try:
					print('STARTING GOAL 3 ... \n')
					is_test_file_created = False
					isFromMap2Check = False
					runNumber = 2
					testCaseFileName = __testSuiteDir__ + "/testcase_2_ES.xml"
					esbmc_command_line = get_command_line(strat, prop, arch, toWorkSourceFile, fp_mode)	
					witness_file_name = os.path.join(INSTRUMENT_OUTPUT_DIR,os.path.basename(benchmark) + '__2.graphml')
					esbmc_command_line += ' --witness-output ' + witness_file_name +' '+'-I'+os.path.dirname(os.path.abspath(benchmark))+ ' '
					if isInstrumentOK : esbmc_command_line += ' --error-label ' + myLabel + ' '
					esbmc_command_line += ' --timeout ' + str(int(time_out_s - 1))+'s '
					esbmc_command_line += ' --memlimit ' + str(MEM_LIMIT_ERROR_CALL_ESBMC) + 'g '
					output = run(esbmc_command_line)
					IsTimeOut(True)
					if os.path.isfile(witness_file_name):
						createTestFile(witness_file_name,toWorkSourceFile, testCaseFileName , False)
						is_test_file_created=True
				except MyTimeOutException as e: raise e
				except KeyboardInterrupt as kbe: raise kbe
				except Exception as ex :
					if IS_DEBUG : print(TColors.FAIL,' Exception', ex , TColors.ENDC)
				
			#res = parse_result(output, category_property)
			
		except MyTimeOutException as e:
			#print('Timeout !!!')
			pass
		except KeyboardInterrupt:
			#print('CTRL + C')
			pass
		#22.06.2020
		if not is_test_file_created: createTestFile(witness_file_name,toWorkSourceFile, testCaseFileName, isFromMap2Check)
		
		if MUST_GENERATE_RANDOM_TESTCASE:
			randomMaxRange = 5
			if map2CheckInputCount > 0:
				randomMaxRange = map2CheckInputCount + 1
				if randomMaxRange == 3:
					randomMaxRange -= 1
					TestCompGenerator([NonDeterministicCall('0')]+\
									[NonDeterministicCall(str(randrange(-128,128))) for i in range(1,randomMaxRange)])\
									.writeTestCase(TESTCASE_FILE_FOR_COVER_ERROR_RANDOM)
					AddFileToArchive(TESTCASE_FILE_FOR_COVER_ERROR_RANDOM,TEST_SUITE_DIR_ZIP)
				elif randomMaxRange == 2:
					TestCompGenerator([NonDeterministicCall(str(singleValueFromMap2Check))])\
									.writeTestCase(TESTCASE_FILE_FOR_COVER_ERROR_RANDOM)
					AddFileToArchive(TESTCASE_FILE_FOR_COVER_ERROR_RANDOM,TEST_SUITE_DIR_ZIP)
				else:		
					randomMaxRange -= 2
					TestCompGenerator([NonDeterministicCall('0')]+\
									[NonDeterministicCall(str(randrange(-128,128))) for i in range(1,randomMaxRange)]+\
									[NonDeterministicCall('0')]).writeTestCase(TESTCASE_FILE_FOR_COVER_ERROR_RANDOM)
					AddFileToArchive(TESTCASE_FILE_FOR_COVER_ERROR_RANDOM,TEST_SUITE_DIR_ZIP)
			
			randomMaxRange = 36
			TestCompGenerator([NonDeterministicCall(str(randrange(-128,128))) for i in range(1,randomMaxRange)]+ \
							[NonDeterministicCall('0')]+[NonDeterministicCall('0')])\
							.writeTestCase(TESTCASE_FILE_FOR_COVER_ERROR_RANDOM2)
			AddFileToArchive(TESTCASE_FILE_FOR_COVER_ERROR_RANDOM2,TEST_SUITE_DIR_ZIP)
			
			randomMaxRange = lastInputInTestcaseCount
			TestCompGenerator([NonDeterministicCall('0')]+\
							[NonDeterministicCall(str(randrange(-128,128))) for i in range(1,randomMaxRange)])\
							.writeTestCase(TESTCASE_FILE_FOR_COVER_ERROR_RANDOM3)
			AddFileToArchive(TESTCASE_FILE_FOR_COVER_ERROR_RANDOM3,TEST_SUITE_DIR_ZIP)
			
			#The New List
			if singleValueFromMap2Check == 0: singleValueFromMap2Check = 128 # No Sigle Value; default 128
			elif singleValueFromMap2Check < 0: singleValueFromMap2Check *= -1
			if IS_DEBUG: 
				print(TColors.OKGREEN,'singleValueFromMap2Check=',singleValueFromMap2Check,TColors.ENDC)
				print(TColors.OKGREEN,'lastInputInTestcaseCount=',lastInputInTestcaseCount,TColors.ENDC)
				print(TColors.OKGREEN,'map2CheckInputCount=',map2CheckInputCount,TColors.ENDC)
			# Do We Need +1 ??
			lst4 = [NonDeterministicCall(str(randrange(-singleValueFromMap2Check,singleValueFromMap2Check))) \
							for i in range(1,lastInputInTestcaseCount)] + \
						[NonDeterministicCall('0'), NonDeterministicCall('0')]
			tmpTestCaseFile = __testSuiteDir__+ '/testcase_4_Fuzzer.xml'
			TestCompGenerator(lst4).writeTestCase(tmpTestCaseFile)
			AddFileToArchive(tmpTestCaseFile,TEST_SUITE_DIR_ZIP)
			
			
		#ZipDir(__testSuiteDir__ ,TEST_SUITE_DIR_ZIP)
		if RUN_COV_TEST:
			RunCovTest() 
		
		#if IS_DEBUG:
			#print(os.times())
		if is_ctrl_c:
			return parse_result("something else will get unknown",prop)
		#Important with False
		#if IsTimeOut(False):
		#	return parse_result("Timed out",prop)
		#return res
		#return parse_result("VERIFICATION SUCCESSFUL",prop)
		if IsTimeOut(False):
			print('The Time is out...')
		return 'DONE'
		
	if prop in [Property.memsafety, Property.overflow, Property.unreach_call, Property.termination, Property.memcleanup]:
		toWorkSourceFile=benchmark # toWorkSourceFile=INSTRUMENT_OUTPUT_FILE
		isFromMap2Check = False		
		try:
			if MUST_RUN_MAP_2_CHECK_FOR_MEM_OVERFLOW_REACH_TERM:
				print('STARTING GOAL 1 ... \n')
				try:
					isFromMap2Check = True
					is_test_file_created = False
					map2CheckNonDetGenerator = 'fuzzer'
					witness_file_name = map2checkWitnessFile
					testCaseFileName = __testSuiteDir__ + '/testcase_map_fuzzer.xml'
					runWithTimeoutEnabled(' '.join(['timeout',str(map2checkTimeMemOverflowReachTerm_Fuzzer)+'s', map2check_path,'--timeout',str(map2checkTimeMemOverflowReachTerm_Fuzzer),'--fuzzer-mb', str(MEM_LIMIT_MEM_OVERFLOW_REACH_TERM_MAP2CHECK), '--nondet-generator', map2CheckNonDetGenerator, '--target-function', '--target-function-name','reach_error','--generate-witness',os.path.abspath(toWorkSourceFile)]), WRAPPER_OUTPUT_DIR)
					if os.path.isfile(map2checkWitnessFile):
						createTestFile(map2checkWitnessFile,toWorkSourceFile, testCaseFileName ,True)
						is_test_file_created = True
				except MyTimeOutException as e: raise e
				except KeyboardInterrupt as kbe: raise kbe
				except Exception as ex :
					if IS_DEBUG : print(TColors.FAIL,' Exception', ex , TColors.ENDC)
					#raise ex
					pass
					
			if mustRunTwice_MemOverflowReachTerm:
				try:
					print('STARTING GOAL 2 ... \n')
					runNumber = 1
					isFromMap2Check = False
					#is_test_file_created = False
					witness_file_name = os.path.join(INSTRUMENT_OUTPUT_DIR, os.path.basename(benchmark) + '_1.graphml')
					testCaseFileName = __testSuiteDir__ + "/testcase_1.xml"
					esbmc_command_line = get_command_line(strat, prop, arch, toWorkSourceFile, fp_mode)
					esbmc_command_line += ' --witness-output ' + witness_file_name +' '+'-I'+os.path.dirname(os.path.abspath(benchmark))+ ' '
					esbmc_command_line += ' --timeout ' + str(int(time_out_s - 1))+'s '
					esbmc_command_line += ' --memlimit ' + str(MEM_LIMIT_ESBMC) + 'g '
					output = run(esbmc_command_line)
					IsTimeOut(True)
					res = parse_result(output, category_property)
					if(res == Result.force_fp_mode):
						fp_mode = True
						esbmc_command_line = get_command_line(strat, prop, arch, toWorkSourceFile, fp_mode)			
						esbmc_command_line += ' --witness-output ' + witness_file_name +' '+'-I'+os.path.dirname(os.path.abspath(benchmark))+ ' '
						#if isInstrumentOK : esbmc_command_line += ' --error-label ' + myLabel + ' '
						esbmc_command_line += ' --timeout ' + str(int(time_out_s - 1))+'s '
						esbmc_command_line += ' --memlimit ' + str(MEM_LIMIT_ERROR_CALL_ESBMC) + 'g '
						output = run(esbmc_command_line)
						IsTimeOut(True)	
					if os.path.isfile(witness_file_name):
						createTestFile(witness_file_name,os.path.abspath(benchmark), testCaseFileName , False)
						is_test_file_created=True
				except MyTimeOutException as e: raise e
				except KeyboardInterrupt as kbe: raise kbe
				except Exception as ex : pass


			try:
				print('STARTING GOAL 3 ... \n')
				runNumber = 2
				isFromMap2Check = False
				is_test_file_created = False
				witness_file_name = os.path.join(INSTRUMENT_OUTPUT_DIR, os.path.basename(benchmark) + '_2.graphml')
				testCaseFileName = __testSuiteDir__ + "/testcase_2.xml"
				esbmc_command_line = get_command_line(strat, prop, arch, toWorkSourceFile, fp_mode)
				esbmc_command_line += ' --witness-output ' + witness_file_name +' '+'-I'+os.path.dirname(os.path.abspath(benchmark))+ ' '
				esbmc_command_line += ' --timeout ' + str(int(time_out_s - 1))+'s '
				esbmc_command_line += ' --memlimit ' + str(MEM_LIMIT_ESBMC) + 'g '
				output = run(esbmc_command_line)
				IsTimeOut(True)
				res = parse_result(output, category_property)
				if(res == Result.force_fp_mode):
					fp_mode = True
					esbmc_command_line = get_command_line(strat, prop, arch, toWorkSourceFile, fp_mode)			
					esbmc_command_line += ' --witness-output ' + witness_file_name +' '+'-I'+os.path.dirname(os.path.abspath(benchmark))+ ' '
					#if isInstrumentOK : esbmc_command_line += ' --error-label ' + myLabel + ' '
					esbmc_command_line += ' --timeout ' + str(int(time_out_s - 1))+'s '
					esbmc_command_line += ' --memlimit ' + str(MEM_LIMIT_ERROR_CALL_ESBMC) + 'g '
					output = run(esbmc_command_line)
					IsTimeOut(True)					
				if os.path.isfile(witness_file_name):
					createTestFile(witness_file_name,toWorkSourceFile, testCaseFileName , False)
					is_test_file_created = True
			except MyTimeOutException as e: raise e
			except KeyboardInterrupt as kbe: raise kbe
			except Exception as ex :
				if IS_DEBUG : print(TColors.FAIL,' Exception', ex , TColors.ENDC)
				#raise ex
				pass

			if RUN_CPA_CHECKER: RunCPAChecker()
			return res
		except MyTimeOutException as e:
			#print('Timeout !!!')
			pass
		except KeyboardInterrupt:
			#print('CTRL + C')
			pass
		except Exception: pass
		if not is_test_file_created: createTestFile(witness_file_name,os.path.abspath(benchmark), testCaseFileName, isFromMap2Check)
		if RUN_CPA_CHECKER: RunCPAChecker()
		if is_ctrl_c:
			return parse_result(output,prop)
		if IsTimeOut(False):
			print('The Time is out...')
		res = parse_result(output, category_property)
		return res

# End of verify mthode

# Options
parser = argparse.ArgumentParser()
parser.add_argument("-a", "--arch", help="Either 32 or 64 bits",type=int, choices=[32, 64], default=32)
parser.add_argument("-v", "--version",help="Prints ESBMC's version", action='store_true')
parser.add_argument("-p", "--propertyfile", help="Path to the property file")
parser.add_argument("benchmark", nargs='?', help="Path to the benchmark")
#parser.add_argument("-s", "--strategy", help="ESBMC's strategy",choices=["kinduction", "falsi", "incr"], default="incr")
parser.add_argument("-z", "--zip_path", help="the tesuite Zip file to generate", default=TEST_SUITE_DIR_ZIP)
parser.add_argument("-t", "--timeout", help="time out seconds",type=float, default=time_out_s)

parser.add_argument("-s", "--strategy", help="ESBMC's strategy", choices=["kinduction", "falsi", "incr", "fixed"], default="fixed")
parser.add_argument("-c", "--concurrency", help="Set concurrency flags", action='store_true')
args = parser.parse_args()

arch = args.arch
version = args.version
property_file = args.propertyfile
benchmark = args.benchmark
strategy = args.strategy
concurrency = args.concurrency

if version:
	#print (os.popen(esbmc_path + "--version").read()[6:]),
	print (FUSEBMC_VERSION)
	exit(0)
if property_file is None:
	print ("Please, specify a property file")
	exit(1)
if benchmark is None:
	print ("Please, specify a benchmark to verify")
	exit(1)

if not args.timeout is None :
	time_out_s = args.timeout
time_out_s -= time_for_zipping_s
if(SHOW_ME_OUTPUT) : print('time_out_s',time_out_s)

if not args.zip_path is None :
	TEST_SUITE_DIR_ZIP = args.zip_path

# Parse property files
f = open(property_file, 'r')
property_file_content = f.read()

if "CHECK( init(main()), LTL(G valid-free) )" in property_file_content:
	category_property = Property.memsafety
elif "CHECK( init(main()), LTL(G valid-memcleanup) )" in property_file_content:
	category_property = Property.memcleanup
elif "CHECK( init(main()), LTL(G ! overflow) )" in property_file_content:
	category_property = Property.overflow
#elif "CHECK( init(main()), LTL(G ! call(__VERIFIER_error())) )" in property_file_content:
elif "CHECK( init(main()), LTL(G ! call(reach_error())) )" in property_file_content:
	category_property = Property.unreach_call
elif "CHECK( init(main()), LTL(F end) )" in property_file_content:
	category_property = Property.termination
#20.05.2020 TODO : remove reach
#elif "COVER( init(main()), FQL(COVER EDGES(@CALL(__VERIFIER_error))) )" in property_file_content:
#	category_property = Property.cover_error_call
elif "COVER( init(main()), FQL(COVER EDGES(@CALL(reach_error))) )" in property_file_content:
	category_property = Property.cover_error_call
#elif "COVER( init(main()), FQL(COVER EDGES(@CALL(__VERIFIER_error))) )" in property_file_content:
#	category_property = Property.unreach_call
elif "COVER( init(main()), FQL(COVER EDGES(@DECISIONEDGE)) )" in property_file_content:
	category_property = Property.cover_branches
else:
	print ("Unsupported Property") 
	exit(1)

#TEST_SUITE_DIR_ZIP_PA='./results-verified/test-comp20_prop-coverage-branches.'+os.path.basename(benchmark)
#if not os.path.isdir(TEST_SUITE_DIR_ZIP_PA):
#	os.makedirs(TEST_SUITE_DIR_ZIP_PA)
#TEST_SUITE_DIR_ZIP=TEST_SUITE_DIR_ZIP_PA+'/test-suite.zip'

if not os.path.isdir(WRAPPER_OUTPUT_DIR):
	os.mkdir(WRAPPER_OUTPUT_DIR)
while True:
	fuSeBMC_run_id = str(GenerateRondomFileName())
	if IS_DEBUG : print('fuSeBMC_run_id=',fuSeBMC_run_id)
	tmpOutputFolder = WRAPPER_OUTPUT_DIR + os.path.basename(benchmark)+'_'+ fuSeBMC_run_id
	if not os.path.isdir(tmpOutputFolder):
		WRAPPER_OUTPUT_DIR = tmpOutputFolder
		os.mkdir(WRAPPER_OUTPUT_DIR)
		break
		
__testSuiteDir__ = WRAPPER_OUTPUT_DIR + "/test-suite/"
META_DATA_FILE = __testSuiteDir__ + "/metadata.xml"
INSTRUMENT_OUTPUT_DIR = WRAPPER_OUTPUT_DIR + '/fusebmc_instrument_output/'

if TEST_SUITE_DIR_ZIP == '':
	TEST_SUITE_DIR_ZIP = WRAPPER_OUTPUT_DIR + '/test-suite.zip'
#fdebug = open(WRAPPER_OUTPUT_DIR + '/fdebug.txt','w')
#fdebug.close()
#os.path.fil
if category_property == Property.cover_branches or category_property == Property.cover_error_call:
	# ESBMC default commands: this is only for Cover-error and cover-branches
	esbmc_dargs = "--no-div-by-zero-check --force-malloc-success --state-hashing "
	#16.05.2020 remove --unlimited-k-steps
	#03.06.2020 kaled reduce the number of "--k-step 120"
	esbmc_dargs += "--no-align-check --k-step 5 --floatbv "
	#02.06.2020 adding options for Coverage-error-call
	# kaled : 03.06.2020 you must put it in method 'get_command_line line 844'; here is general
	#esbmc_dargs += "--no-align-check --k-step 120 --floatbv --unlimited-k-steps "
	esbmc_dargs += "--context-bound 2 "
	#--unwind 1000 --max-k-step 1000 
	esbmc_dargs += "--show-cex "
	#esbmc_dargs += " --overflow-check " 
	
	INSTRUMENT_OUTPUT_FILE = WRAPPER_OUTPUT_DIR+'/fusebmc_instrument_output/instrumented.c'
	INSTRUMENT_OUTPUT_FILE_OBJ = WRAPPER_OUTPUT_DIR+'/fusebmc_instrument_output/instrumented.o'
	INSTRUMENT_OUTPUT_GOALS_FILE = WRAPPER_OUTPUT_DIR+'/fusebmc_instrument_output/goals.txt'
	INSTRUMENT_OUTPUT_GOALS_DIR = WRAPPER_OUTPUT_DIR+'/fusebmc_instrument_output/goals_output/'

	TESTCASE_FILE_FOR_COVER_ERROR = __testSuiteDir__ + "/testcase_1_ES.xml"
	TESTCASE_FILE_FOR_COVER_ERROR_RANDOM = __testSuiteDir__ + "/testcase_1_Fuzzer.xml"
	TESTCASE_FILE_FOR_COVER_ERROR_RANDOM2 = __testSuiteDir__ + "/testcase_2_Fuzzer.xml"
	TESTCASE_FILE_FOR_COVER_ERROR_RANDOM3 = __testSuiteDir__ + "/testcase_3_Fuzzer.xml"
	TESTCASE_FILE_FOR_MAP_CHECK = __testSuiteDir__ + "/testcase_map.xml"

	MakeFolderEmptyORCreate(INSTRUMENT_OUTPUT_DIR)
	RemoveFileIfExists(INSTRUMENT_OUTPUT_FILE)
	if category_property == Property.cover_branches:
		RemoveFileIfExists(INSTRUMENT_OUTPUT_GOALS_FILE)
		if MUST_RUN_MAP_2_CHECK_FOR_BRANCHES:
			map2checkWitnessFile= WRAPPER_OUTPUT_DIR + '/witness.graphml'
		if FuSeBMCFuzzerLib_COVERBRANCHES_ENABLED:
			FuSeBMCFuzzerLib_COVERBRANCHES_SEED_DIR = os.path.abspath(os.path.join(WRAPPER_OUTPUT_DIR, 'seeds'))
			MakeFolderEmptyORCreate(FuSeBMCFuzzerLib_COVERBRANCHES_SEED_DIR)
			FuSeBMCFuzzerLib_COVERBRANCHES_COVERED_GOALS_FILE = os.path.abspath(WRAPPER_OUTPUT_DIR+'/FuSeBMC_Fuzzer_goals_covered.txt')
		#MakeFolderEmptyORCreate(INSTRUMENT_OUTPUT_GOALS_DIR)
	MakeFolderEmptyORCreate(__testSuiteDir__)
	
	if category_property == Property.cover_error_call:
		if MUST_RUN_MAP_2_CHECK_FOR_ERROR_CALL_FUZZER or MUST_RUN_MAP_2_CHECK_FOR_ERROR_CALL_SYMEX:
			map2checkWitnessFile= WRAPPER_OUTPUT_DIR + '/witness.graphml'
			#RemoveFileIfExists(map2checkWitnessFile)
	
	RemoveFileIfExists(TEST_SUITE_DIR_ZIP)
	WriteMetaDataFromWrapper()
	AddFileToArchive(META_DATA_FILE,TEST_SUITE_DIR_ZIP)
	
	if not os.path.isfile(FUSEBMC_INSTRUMENT_EXE_PATH) and category_property == Property.cover_branches:
		print("FuSeBMC_inustrument cannot be found..")
		#exit(1)
	benchmarkbase=os.path.basename(benchmark)
	#strategy
	#benchmarkbase in ["sum02-1.c"] and 
	result = verify(strategy, category_property, False)
	#print(get_result_string(result))
	print(result)
	exit(0)
	
	#assumptionParser=AssumptionParser('/home/kaled/counter_example/GOAL_2.graphml');
	#assumptionParser.parse()
	#print(assumptionParser.assumptions)
	#for ass in assumptionParser.assumptions:
	#	ass.debugInfo()
	#exit(1)
elif category_property in [Property.memsafety ,Property.overflow, Property.unreach_call,Property.termination, Property.memcleanup]:
	
	# ESBMC default commands: this is the same for : memory, overflow , reach, termination
	esbmc_dargs = "--no-div-by-zero-check --force-malloc-success --state-hashing "
	esbmc_dargs += "--no-align-check --k-step 2 --floatbv --unlimited-k-steps "
	esbmc_dargs += "--no-por --context-bound-step 5 --max-context-bound 15 "
	if concurrency:
		esbmc_dargs += "--incremental-cb --context-bound-step 5 "
		esbmc_dargs += "--unwind 8 "
		esbmc_dargs += "--no-slice " # TODO: Witness validation is only working without slicing
		# NOTE: max-context-bound and no-por are already in the default params
	
	MakeFolderEmptyORCreate(INSTRUMENT_OUTPUT_DIR)
	MakeFolderEmptyORCreate(__testSuiteDir__)
	WriteMetaDataFromWrapper()
	AddFileToArchive(META_DATA_FILE,TEST_SUITE_DIR_ZIP)
	result = verify(strategy, category_property, False)
	print (get_result_string(result))
	exit(0)
else:
	print(category_property , ' is Unkown...!!')
	exit(0)

#witness_file_name = os.path.basename(benchmark) + ".graphml"
#if not os.path.exists(__testSuiteDir__):
#	os.mkdir(__testSuiteDir__)
#createTestFile(witness_file_name, benchmark)
#ZipDir(__testSuiteDir__ ,TEST_SUITE_DIR_ZIP)
