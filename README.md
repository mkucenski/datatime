# datatime
A recreation of of The SleuthKit's 'mactime' with a goal of faster results and allowing integration of timeline data from other sources (EVT, LNK, Firewall, etc.). 

## License
<p>Copyright &copy; 2016 Matthew A. Kucenski</p>

<p>Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0</p>

<p>Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.</p>

## Description
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

