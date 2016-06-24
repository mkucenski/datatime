# timeCorrelator
A recreation of of The SleuthKit's 'mactime' with a goal of allowing integration of timeline data from other sources (EVT, LNK, Firewall, etc.)

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

