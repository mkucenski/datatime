// Copyright 2007 Matthew A. Kucenski
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//#define _DEBUG_ 1

#include <iostream>
#include <popt.h>
#include <string>
#include <vector>
#include <map>
using namespace std;

#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/lexical_cast.hpp"
using namespace boost;

#include "libdelimText/src/delimTextFile.h"
#include "libtimeUtils/src/timeUtils.h"
#include "libtimeUtils/src/timeZoneCalculator.h"
#include "misc/debugMsgs.h"
#include "misc/poptUtils.h"
#include "misc/tsk_mactime.h"

int main(int argc, const char** argv) {
	int rv = EXIT_FAILURE;
	
	vector<string> filenameVector;
	bool bDelimited = false;
	bool bMactime = false;
	char chDelim = '|';
	char chQualifier = '\0';
	bool bAllFields = false;
	gregorian::date startDate(gregorian::min_date_time);
	gregorian::date endDate(gregorian::max_date_time);

    // The retrieved entries are entered into the multimap using the time value as the 'key'. The multimap is automatically sorted based on those values; output is therefore sorted in ascending time order.
	multimap<long, delimTextRow*> timeToRecordMap;

    // TODO I don't recall why it would be necessary to load all of the rows into a vector and then delete them all at the end...
    //vector<delimTextRow*> allRows;
    
	timeZoneCalculator tzcalc;
	int iTrimData = -1;
	
	struct poptOption optionsTable[] = {
		{"field-separator",	't',	POPT_ARG_STRING,	NULL,	10,	"Input body file field separator.  Defaults to '|'.",	"separator"},
		{"mactime",				'm',	POPT_ARG_NONE,		NULL,	15,	"Output in the SleuthKit's mactime format.", NULL},
		{"delimited",			'd',	POPT_ARG_NONE,		NULL,	20,	"Output in comma-delimited format.",	NULL},
		{"timezone",			'z',	POPT_ARG_STRING,	NULL,	30,	"POSIX timezone string (e.g. 'EST-5EDT,M4.1.0,M10.1.0' or 'GMT-5') to be used when displaying data. Defaults to GMT.", "zone"},
		{"allfields",			'a',	POPT_ARG_NONE,		NULL,	40,	"Display all data fields.  Useful when working with custom data sources. Only applicable in comma-delimited mode.", NULL},
		//{"qualifier",			'q',	POPT_ARG_STRING,	NULL,	50,	"Input field qualifier. Defaults to '\"'. (e.g. ...,field0,\"fie,ld1\",field2,...)", "character"},
		{"trim-data",			 0,		POPT_ARG_INT,		NULL,	60,	"Trim data field for easier viewing. Use caution when searching as your are trimming potentially relevent data. Not applicable in comma-delimited mode.", "characters"},
		{"start-date",			 0,		POPT_ARG_STRING,	NULL,	70, 	"Only display entries recorded after the specified date.", "yyyy-mm-dd"},
		{"end-date", 			 0,		POPT_ARG_STRING,	NULL,	80, 	"Only display entries recorded before the specified date.", "yyyy-mm-dd"},
		{"version",	 			 0,		POPT_ARG_NONE,		NULL,	100,	"Display version.", NULL},
		POPT_AUTOHELP
		POPT_TABLEEND
	};
	poptContext optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);
	poptSetOtherOptionHelp(optCon, "");

	if (argc < 1) {
		usage(optCon);
		exit(EXIT_FAILURE);
	}

	string strTmp;
	int iOption = poptGetNextOpt(optCon);
	while (iOption >= 0) {
		switch (iOption) {
			case 10:
				chDelim = poptGetOptArg(optCon)[0];
				break;
			case 15:
				bMactime = true;
				break;
			case 20:
				bDelimited = true;
				break;
			case 30:
				if (tzcalc.setTimeZone(poptGetOptArg(optCon)) >= 0) {
				} else {
					usage(optCon, "Invalid time zone string", "e.g. 'EST-5EDT,M4.1.0,M10.1.0' or 'GMT-5'");
					exit(EXIT_FAILURE);
				}
				break;
			case 40:
				bAllFields = true;
				break;
			case 50:
				strTmp = poptGetOptArg(optCon);
				if (strTmp.length() == 1) {
					chQualifier = strTmp[0];
				} else {
					usage(optCon, "Invalid field qualifier", "e.g. \"");
					exit(EXIT_FAILURE);
				}
				break;
			case 60:
				iTrimData = strtol(poptGetOptArg(optCon), NULL, 10);
				break;
			case 70:
				strTmp = poptGetOptArg(optCon);
				if (strTmp.length() == 10) {
					startDate = gregorian::from_string(strTmp);
					DEBUG_INFO(PACKAGE << ": Start Date = " << startDate);
				} else {
					usage(optCon, "Invalid start date value", "e.g. yyyy-mm-dd");
					exit(EXIT_FAILURE);
				}
				break;
			case 80:
				strTmp = poptGetOptArg(optCon);
				if (strTmp.length() == 10) {
					endDate = gregorian::from_string(strTmp);
					DEBUG_INFO(PACKAGE << ": End Date = " << endDate);
				} else {
					usage(optCon, "Invalid end date value", "e.g., yyyy-mm-dd");
					exit(EXIT_FAILURE);
				}
				break;
			case 100:
				version(PACKAGE, VERSION);
				exit(EXIT_SUCCESS);
				break;
		}
		iOption = poptGetNextOpt(optCon);
	}
	
	gregorian::date_period dateRange(startDate, endDate + gregorian::date_duration(1));	//Add an additional day to be inclusive of the given end date

	if (iOption != -1) {
		usage(optCon, poptBadOption(optCon, POPT_BADOPTION_NOALIAS), poptStrerror(iOption));
		exit(EXIT_FAILURE);
	}

    // Read all of the remainging arguments as "body" filenames.
	const char* cstrFilename = poptGetArg(optCon);
	while (cstrFilename) {
		filenameVector.push_back(cstrFilename);
		cstrFilename = poptGetArg(optCon);
	}
	
	if (filenameVector.size() < 1) {
		filenameVector.push_back("");		//If no files are given, an empty filename will cause libDelimFile to read from stdin
	}
	
    // For each filename, open a delimTextFile object and iterate through the rows.
	for (vector<string>::iterator it = filenameVector.begin(); it != filenameVector.end(); it++) {
		DEBUG_INFO(PACKAGE << ": Reading file: " << *it);
		delimTextFile delimFileObj(*it, chDelim, chQualifier);
		
		while (true) {
			delimTextRow* pDelimRowObj = new delimTextRow;

			// TODO - Is this necessary?
			//allRows.push_back(pDelimRowObj);
			
			long lMTime, lATime, lCTime, lCRTime;
			if (delimFileObj.getNextRow(pDelimRowObj)) {				
				DEBUG_INFO(PACKAGE << ": Retrieved Row: " << pDelimRowObj->getField(TSK3_MACTIME_NAME));

				lMTime = -1;
				lATime = -1;
				lCTime = -1;
				lCRTime = -1;
				pDelimRowObj->getFieldAsLong(TSK3_MACTIME_MTIME, &lMTime);
				pDelimRowObj->getFieldAsLong(TSK3_MACTIME_ATIME, &lATime);
				pDelimRowObj->getFieldAsLong(TSK3_MACTIME_CTIME, &lCTime);
				pDelimRowObj->getFieldAsLong(TSK3_MACTIME_CRTIME, &lCRTime);
				
				DEBUG_INFO(PACKAGE << ": Loading records, MTime = " << lMTime << ", ATime = " << lATime << ", CTime = " << lCTime << ", CRTime = " << lCRTime);
				if (lMTime == -1 && lATime == -1 && lCTime == -1 && lCRTime == -1) {	//If there are no valid dates, the row gets automatically added with -1
					timeToRecordMap.insert(pair<long, delimTextRow*>(-1, pDelimRowObj));
				} else {    //If there are valid dates, the row is subject to date range rules
					if (lMTime >= 0) {
						if (dateRange.contains(tzcalc.calculateLocalTime(posix_time::from_time_t(lMTime)).local_time().date())) {
							timeToRecordMap.insert(pair<long, delimTextRow*>(lMTime, pDelimRowObj));
						}
					}
					
					if (lATime >= 0 && lATime != lMTime) {  //Only add a row more than once if the various times are different from each other.
						if (dateRange.contains(tzcalc.calculateLocalTime(posix_time::from_time_t(lATime)).local_time().date())) {
							timeToRecordMap.insert(pair<long, delimTextRow*>(lATime, pDelimRowObj));
						}
					}
										
					if (lCTime >= 0 && lCTime != lMTime && lCTime != lATime) {  //Only add a row more than once if the various times are different from each other.
						if (dateRange.contains(tzcalc.calculateLocalTime(posix_time::from_time_t(lCTime)).local_time().date())) {
							timeToRecordMap.insert(pair<long, delimTextRow*>(lCTime, pDelimRowObj));
						}
					}
										
					if (lCRTime >= 0 && lCRTime != lMTime && lCRTime != lATime && lCRTime != lCTime) {  //Only add a row more than once if the various times are different from each other.
						if (dateRange.contains(tzcalc.calculateLocalTime(posix_time::from_time_t(lCRTime)).local_time().date())) {
							timeToRecordMap.insert(pair<long, delimTextRow*>(lCRTime, pDelimRowObj));
						}
					}
				}
			} else {
				break;
			}
		}	//while () {
	}	//for (vector<string>::iterator it = filenameVector.begin(); it != filenameVector.end(); it++) {

	if (!bMactime) {
		cout << "Time Zone: \"" << tzcalc.getTimeZoneString() << "\"" << endl;		//Display the timezone so that the reader knows which zone was used for this output
	}

	long lastTime = -1;
	long lMTime, lATime, lCTime, lCRTime;
	for(multimap<long, delimTextRow*>::iterator it = timeToRecordMap.begin(); it != timeToRecordMap.end(); it++) {
		lMTime = -1;
		lATime = -1;
		lCTime = -1;
		lCRTime = -1;

		it->second->getFieldAsLong(TSK3_MACTIME_MTIME, &lMTime);
		it->second->getFieldAsLong(TSK3_MACTIME_ATIME, &lATime);
		it->second->getFieldAsLong(TSK3_MACTIME_CTIME, &lCTime);
		it->second->getFieldAsLong(TSK3_MACTIME_CRTIME, &lCRTime);
		
		string strFields[11];
		it->second->getField(TSK3_MACTIME_NAME, &strFields[TSK3_MACTIME_NAME]);
		it->second->getField(TSK3_MACTIME_MD5, &strFields[TSK3_MACTIME_MD5]);
		it->second->getField(TSK3_MACTIME_SIZE, &strFields[TSK3_MACTIME_SIZE]);
		it->second->getField(TSK3_MACTIME_PERMS, &strFields[TSK3_MACTIME_PERMS]);
		it->second->getField(TSK3_MACTIME_UID, &strFields[TSK3_MACTIME_UID]);
		it->second->getField(TSK3_MACTIME_GID, &strFields[TSK3_MACTIME_GID]);
		it->second->getField(TSK3_MACTIME_INODE, &strFields[TSK3_MACTIME_INODE]);

		if (bDelimited) {

		
			cout 	<< (it->first >= 0 ? getDateTimeString(tzcalc.calculateLocalTime(posix_time::from_time_t(it->first))) : "Unknown") << ","
					<< strFields[TSK3_MACTIME_MD5] << ","
					<< strFields[TSK3_MACTIME_SIZE] << ","
					<< (lMTime == it->first ? 'm' : '.') << (lATime == it->first ? 'a' : '.') << (lCTime == it->first ? 'c' : '.') << (lCRTime == it->first ? 'b' : '.') << ","
					<< strFields[TSK3_MACTIME_PERMS] << ","
					<< strFields[TSK3_MACTIME_UID] << ","
					<< strFields[TSK3_MACTIME_GID] << ","
					<< strFields[TSK3_MACTIME_INODE] << ","
					<< strFields[TSK3_MACTIME_NAME]
					<< "\n";

		} else if (bMactime) {
			DEBUG_INFO(PACKAGE << " [bMactime] it->first=" << it->first << " lATime=" << lATime << " lMTime=" << lMTime << " lCTime=" << lCTime << " lCRTime=" << lCRTime);

			//Sleuthkit TSK3.x body format
			//0  |1        |2    |3     |4       |5       |6   |7    |8    |9    |10
			//MD5|NAME     |INODE|PERMS |UID     |GID     |SIZE|ATIME|MTIME|CTIME|CRTIME
		
			cout 	<< strFields[TSK3_MACTIME_MD5] << "|"
					<<	strFields[TSK3_MACTIME_NAME] << "|"
					<< strFields[TSK3_MACTIME_INODE] << "|"
					<< strFields[TSK3_MACTIME_PERMS] << "|"
					<< strFields[TSK3_MACTIME_UID] << "|"
					<< strFields[TSK3_MACTIME_GID] << "|"
					<< strFields[TSK3_MACTIME_SIZE] << "|";

			cout 	<< (lATime == it->first ? it->second->getField(TSK3_MACTIME_ATIME) : "") << "|"
					<< (lMTime == it->first ? it->second->getField(TSK3_MACTIME_MTIME) : "") << "|"
					<< (lCTime == it->first ? it->second->getField(TSK3_MACTIME_CTIME) : "") << "|"
					<< (lCRTime == it->first ? it->second->getField(TSK3_MACTIME_CRTIME) : "") << "\n";
			
		} else {

			//TODO For non-delimited output, dynamically size rows based on maximum text width
			DEBUG_INFO(PACKAGE << ": it->first time value = " << it->first);
			//cout.fill('_');

			if (it->first != lastTime) {								//Date-Time
				cout.width(24);
				cout << (it->first >= 0 ? getDateTimeString(tzcalc.calculateLocalTime(posix_time::from_time_t(it->first))) : "Unknown Date/Time");
				lastTime = it->first;
			} else {    //Don't repeat the same date over and over
				cout.width(24);
				cout << "";
			}
			cout << " ";

			cout.width(10);
			cout << strFields[TSK3_MACTIME_SIZE];
			cout << " ";

			cout << (lMTime == it->first ? 'm' : '.') << (lATime == it->first ? 'a' : '.') << (lCTime == it->first ? 'c' : '.') << (lCRTime == it->first ? 'b' : '.');
			cout << " ";

			cout.width(12);
			cout << strFields[TSK3_MACTIME_PERMS];
			cout << " ";

			cout.width(8);
			cout << strFields[TSK3_MACTIME_UID];
			cout << " ";

			cout.width(8);
			cout << strFields[TSK3_MACTIME_GID];
			cout << " ";

			cout.width(12);
			cout << strFields[TSK3_MACTIME_INODE];
			cout << " ";

			if (iTrimData >= 0) {
				cout << string(strFields[TSK3_MACTIME_NAME], 0, iTrimData);
			} else {
				cout << strFields[TSK3_MACTIME_NAME];
			}
			cout << "\n";

		}	//if (bDelimited) {
	}	//for(multimap<long, string*>::iterator it = dateToRecordMap.begin(); it != dateToRecordMap.end(); it++) {

    // TODO - ?
	//for(vector<delimTextRow*>::iterator it = allRows.begin(); it != allRows.end(); it++) {
	//	delete *it;
	//}
	//allRows.clear();

	exit(rv);	
}	//int main(int argc, const char** argv) {
