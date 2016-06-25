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

int main(int argc, const char** argv) {
	int rv = EXIT_FAILURE;
	
	vector<string> filenameVector;
	bool bDelimited = false;
	char chDelim = '|';
	char chQualifier = '"';
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
		{"field-separator",     't',	POPT_ARG_STRING,	NULL,	10,	"Input body file field separator.  Defaults to '|'.",	"separator"},
		{"delimited",			'd',	POPT_ARG_NONE,		NULL,	20,	"Output in comma-delimited format.",	NULL},
		{"timezone", 			'z',	POPT_ARG_STRING,	NULL,	30,	"POSIX timezone string (e.g. 'EST-5EDT,M4.1.0,M10.1.0' or 'GMT-5') to be used when displaying data. Defaults to GMT.", "zone"},
		{"allfields",			'a',	POPT_ARG_NONE,		NULL,	40,	"Display all data fields.  Useful when working with custom data sources. Only applicable in comma-delimited mode.", NULL},
		{"qualifier",			'q',	POPT_ARG_STRING,	NULL,	50,	"Input field qualifier. Defaults to '\"'. (e.g. ...,field0,\"fie,ld1\",field2,...)", "character"},
		{"trim-data",			0,		POPT_ARG_INT,		NULL,	60,	"Trim data field for easier viewing. Use caution when searching as your are trimming potentially relevent data. Not applicable in comma-delimited mode.", "characters"},
		{"start-date",          0,		POPT_ARG_STRING,	NULL,	70, 	"Only display entries recorded after the specified date.", "yyyy-mm-dd"},
		{"end-date", 			0,		POPT_ARG_STRING,	NULL,	80, 	"Only display entries recorded before the specified date.", "yyyy-mm-dd"},
		{"version",	 			0,		POPT_ARG_NONE,		NULL,	100,	"Display version.", NULL},
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
					DEBUG_INFO(PACKAGE << " Start Date = " << startDate);
				} else {
					usage(optCon, "Invalid start date value", "e.g. yyyy-mm-dd");
					exit(EXIT_FAILURE);
				}
				break;
			case 80:
				strTmp = poptGetOptArg(optCon);
				if (strTmp.length() == 10) {
					endDate = gregorian::from_string(strTmp);
					DEBUG_INFO(PACKAGE << " End Date = " << endDate);
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
		delimTextFile delimFileObj(*it, chDelim, chQualifier);
		
		while (true) {
			delimTextRow* pDelimRowObj = new delimTextRow;

            // TODO - Is this necessary?
			//allRows.push_back(pDelimRowObj);
			
			long lMTime, lATime, lCTime;
			if (delimFileObj.getNextRow(pDelimRowObj)) {				
				lMTime = -1;
				lATime = -1;
				lCTime = -1;
				pDelimRowObj->getFieldAsLong(12, &lMTime);
				pDelimRowObj->getFieldAsLong(11, &lATime);
				pDelimRowObj->getFieldAsLong(13, &lCTime);
				
				DEBUG_INFO(PACKAGE << " Loading records, MTime = " << lMTime << ", ATime = " << lATime << ", CTime = " << lCTime);
				if (lMTime == -1 && lATime == -1 && lCTime == -1) {	//If there are no valid dates, the row gets automatically added with -1
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
				}
			} else {
				break;
			}
		}	//while () {
	}	//for (vector<string>::iterator it = filenameVector.begin(); it != filenameVector.end(); it++) {

	cout << "Time Zone: \"" << tzcalc.getTimeZoneString() << "\"" << endl;		//Display the timezone so that the reader knows which zone was used for this output
		
	long lastTime = -1;
	long lMTime, lATime, lCTime;
	for(multimap<long, delimTextRow*>::iterator it = timeToRecordMap.begin(); it != timeToRecordMap.end(); it++) {
		lMTime = -1;
		lATime = -1;
		lCTime = -1;
		it->second->getFieldAsLong(12, &lMTime);
		it->second->getFieldAsLong(11, &lATime);
		it->second->getFieldAsLong(13, &lCTime);
		
		string strFields[11];
		if (it->second->getField(0, &strFields[0]) &&
			it->second->getField(10, &strFields[10]) &&
			it->second->getField(5, &strFields[5]) &&
			it->second->getField(7, &strFields[7]) &&	//uid
			it->second->getField(8, &strFields[8]) &&	//gid
			it->second->getField(3, &strFields[3]) &&
			it->second->getField(1, &strFields[1])) {
				
			if (bDelimited && bAllFields) {
				if (it->second->getField(2, &strFields[2]) &&
					it->second->getField(4, &strFields[4]) &&
					it->second->getField(6, &strFields[6]) &&
					it->second->getField(9, &strFields[9])) {
				} else {
					cerr << "ERROR: Unable to retrieve additional field values.\n";
				}
			}
		} else {
			cerr << "ERROR: Unable to retrieve base field values.\n";
		}
			
		if (bDelimited) {
			cout 	<< (it->first >= 0 ? getDateTimeString(tzcalc.calculateLocalTime(posix_time::from_time_t(it->first))) : "Unknown") << ","
					<< strFields[0] << ","
					<< strFields[10] << ","
					<< (lMTime == it->first ? 'm' : '.') << (lATime == it->first ? 'a' : '.') << (lCTime == it->first ? 'c' : '.') << ","
					<< strFields[5] << ","
					<< strFields[7] << ","
					<< strFields[8] << ","
					<< strFields[3] << ","
					<< strFields[1];

			if (bAllFields == true) {
				cout 	<< "," << strFields[2]
						<< "," << strFields[4]
						<< "," << strFields[6]
						<< "," << strFields[9];

				string strField;
				unsigned int field = 14;
				while (it->second->getField(field, &strField)) {
					cout << "," << strField;
					field++;
				}				
			}	//if (bAllFields == true) {
								
			cout << "\n";
		} else {
			//TODO For non-delimited output, dynamically size rows based on maximum text width
			
			DEBUG_INFO(PACKAGE << " it->first time value = " << it->first);
			if (it->first != lastTime) {								//Date-Time
				cout.width(24);
				cout << (it->first >= 0 ? getDateTimeString(tzcalc.calculateLocalTime(posix_time::from_time_t(it->first))) : "Unknown Date/Time") << " ";
				lastTime = it->first;
			} else {    //Don't repeat the same date over and over
				cout.width(24);
				cout << " " << " ";
			}
			cout.width(3);
			cout << strFields[0] << " ";    //Type (i.e. EVT,LNK,FWL,etc)
			cout.width(15);
			cout << strFields[10] << " ";   //Size
			cout << (lMTime == it->first ? 'm' : '.') << (lATime == it->first ? 'a' : '.') << (lCTime == it->first ? 'c' : '.') << " ";
			cout.width(15);
			cout << strFields[5] << " ";    //Permissions
			cout.width(15);
			cout << strFields[7] << " ";    //UID
			cout.width(15);
			cout << strFields[8] << " ";    //GID
			cout.width(15);
			cout << strFields[3] << " ";    //INODE
			if (iTrimData >= 0) {           //FILE
				cout << string(strFields[1], 0, iTrimData);
			} else {
				cout << strFields[1];
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
