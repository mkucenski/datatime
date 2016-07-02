datatime
========
A recreation of of The SleuthKit's 'mactime' with a goal of faster results and allowing integration of timeline data from other sources (EVT, LNK, Firewall, etc.). 

NOTE: While this code compiles, it was written several years ago based on an older version of The SleuthKit's "body" format. It needs to be udated based on the TSK3.x format.

License
-------
<p>Copyright &copy; 2016 Matthew A. Kucenski</p>

<p>Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0</p>

<p>Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.</p>

Description
-----------

The goal of this project was/is to use TSK's body format to integrate timeline data from other sources such as firewall logs, log records, shortcut files, network data (PCAP), etc. to focus and visualize combined events to understand what was happening at a specific time.

While I'm not convinced the TSK "body" format is really the best or most flexible format to use for this application, it provided an easy and acceptable solution at the time I wrote it.

Another goal of this project was to increase the speed of sorting/processing the records and providing usable results to the analyst. It has been many years since I conducted any tests, but as I recall this application was able to process records *much* faster--a requirement when you start adding massive amounts of firewall, network, and other data on top of already vast filesystem records.

Formatting Notes
----------------
//Sleuthkit TSK3.x body format - TODO - Not yet adjusted and implemented...
//0  |1   |2    |3          |4  |5  |6   |7    |8    |9    |10    |11     |12     |13       |14     |15
//MD5|NAME|INODE|PERMISSIONS|UID|GID|SIZE|ATIME|MTIME|CTIME|CRTIME


//Sleuthkit TSK2.x body format - Maintain this format for easy compatibility with fls/ils
//0  |1        |2     |3    |4       |5       |6    |7  |8  |9   |10  |11   |12      |13   |14      |15
//MD5|PATH/NAME|DEVICE|INODE|PERM-VAL|PERM-STR|LINKS|UID|GID|RDEV|SIZE|ATIME|MTIME   |CTIME|BLK-SIZE|BLKS

//Normal mactime for files
//   |FILENAME |      |INODE|        |PERM-STR|     |UID|GID|     |SIZE|ATIME|MTIME  |CTIME

//Mactime for event records
//EVT|EVENTFILE|REC#  |EVTID|        |SOURCE  |     |SID|PC |     |TYPE|     |WRITTEN|GENERATED

//Mactime for firewall records
//FWL|URL      |      |     |        |TYPE    |     |   |   |     |    |TIME

//Mactime for lnk files
//LNK|SHORTCUT |      |     |        |        |     |   |   |     |SIZE|ATIME|MTIME  |CTIME

