/*
 ****************************************************************************
 ****************************************************************************
 * Copyright (c) 2008 Jan Frohn. All rights reserved.                       *
 ****************************************************************************
 * This program may be distributed and/or modified                          *
 * under the terms of the GNU General Public License version 2 as published *
 * by the Free Software Foundation and appearing in the file gpl.txt        *
 * included in the packaging of this file.                                  *
 * This program is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRANTY; without even the implied warranty of               *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                     *
 * See the GNU General Public License for more details.                     *
 ****************************************************************************
 ****************************************************************************
 * Dieses Programm und saemtliche Dateien sind urheberrechtlich geschuetzt. *
 * Der Inhaber aller Rechte an diesem Programm ist Jan Frohn.               *
 * (c) 2008, Jan Frohn                                                      *
 ****************************************************************************
 * Dieses Programm kann unter den Bedingungen der GNU General Public        *
 * License Version 2, wie von der Free Software Foundation veroeffentlicht, *
 * weitergeben und/oder modifiziert werden. Die englischsprachige           * 
 * Originalversion der Lizenz kann der Datei gpl.txt entnommen werden, die  *
 * dem Packet beiliegt.                                                     *
 * Die Veroeffentlichung dieses Programms erfolgt in der Hoffnung, dass es  *
 * Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, sogar ohne    *
 * die implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT F�R EINEN  *
 * BESTIMMTEN ZWECK. Details finden Sie in der GNU General Public License.  *
 ****************************************************************************
 ****************************************************************************
 */

//#include <termios.h> 
#include "unistd.h"
//#include "getopt.h"
#include <errno.h>
#include <fcntl.h>
//#include <sys/time.h>
//#include <sys/stat.h>
#include <cstdlib>
#include <cstring>
#include<iostream>
#include<iomanip>


#include "cgidata.h"
#include "utils.h"
#include "datadisplay.h"

using namespace std;

void getOptions(int argc, char * const argv[], DDOptions &opt){
  int c;
  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
      {"no-packets",    0, 0, 'p'},
      {"no-statistics", 0, 0, 's'},
      {"no-unsynced",   0, 0, 'u'},
      {"summarize",     0, 0, 'e'},
      {"columns",       0, 0, 'c'},
      {"realtime",      0, 0, 'r'},
      {"hex",           0, 0, 'x'},
      {"verbose",       0, 0, 'v'},
      {"help",          0, 0, 'h'},
      {"device",        1, 0, 'd'},
      {0,               0, 0, 0}
    };

    c = getopt_long (argc, argv, "crhvd:xpsue", long_options, &option_index);
    if (c == -1) break;

    switch (c) {
      case 'c':
        opt.formatColumns =true;
        if(opt.verbose) cout<<"\noption \"-c\"";
        break;

      case 'r':
        opt.formatRealTimeCSV =true;
        if(opt.verbose) cout<<"\noption \"-r\"";
        break;

      case 'x':
        opt.formatHex = true;
        if(opt.verbose) cout<<"\noption \"-x\"";
        break;

      case 'p':
        opt.printPackets = false;
        if(opt.verbose) cout<<"\noption \"-p\"";
        break;

      case 's':
        opt.printStatistics = false;
        if(opt.verbose) cout<<"\noption \"-s\"";
        break;

      case 'u':
        opt.printUnsyncedChars = false;
        if(opt.verbose) cout<<"\noption \"-u\"";
        break;

      case 'e':
        opt.summarize = true;
        if(opt.verbose) cout<<"\noption \"-e\"";
        break;

      case 'v':
        opt.verbose = true;
        if(opt.verbose) cout<<"\noption \"-v\"";
        break;


      case 'd':
        opt.device = optarg;
        if(opt.verbose) cout<<"\noption \"-d\" with arg "<<optarg;
       break;

      case 'h':
        cout<<
        "\ndatadisplay is a program to display the data export of TECHNODOLLY camera cranes"
        "\nin a human-readable fashion. The output is written to standard output."
        "\nSee written documentation for details and examples."
        "\nCopyright (c) 2008 by Jan Frohn."
        "\nSend bug reports or comments to jan.frohn@supertechno.com."
        "\n"
        "\nUsage: datadisplay [OPTIONS]\n"
        "\nOPTIONS"
        "\n"
        "\n1. general options:"
        "\n -h, --help           show this help screen"
        "\n -v, --verbose        increase verbosity"
        "\n -d, --device DEV     read from character device or file DEV. Defaults to /dev/ttyS0."
        "\n -s, --no-statistics  don't display some statistical information for each packet."
        "\n -p, --no-packets     don't display packet data."
        "\n -u, --no-unsynced    don't display characters when the receiver is out of sync."
        "\n -e, --summarize      display a summary at the end (start frame, end frame, etc.)."
        "\n -x, --hex            show floats as hex values. Good for debugging."
        "\n -c, --columns        ouput raw column based text. Each packet generates a line"
        "\n                      and each field corresponds to a column. Good for interfacing to" 
        "\n                      external programs. The order of columns corresponds to"
        "\n                      the order of fields in CGIDataCartesian."
        "\n -r, --realtime       ouput raw column based text equivalent to the ASCII real" 
        "\n                      time data string (comma separated). Use this to convert binary"
        "\n                      to ASCII data."
        "\n"<<flush;
        exit(0);
        break;

      case '?'://An error message is already generated internally!
        exit(-1);
        break;

      default:
        cout<<"?? getopt returned character code 0x"<<std::hex<<c<<std::dec<<" ??\n";
    }
  }

  if(optind < argc) {
    cout<<"\nNon-option arguments found:";
    while (optind < argc){
      cout<<"\n"<<argv[optind++];
    }
    cout<<"\nExiting!";
    exit(-1);
  }
}

void printStatistics(std::ostream &o, const dataReceptionStatus &DRS){
  o<<std::dec<<"\n------------------------------";
  o<<"\ntotal chars read: "<<DRS.totalCharsRead<<"  (equivalent to "
   <<DRS.totalCharsRead / (double) CGI_DATA_LENGTH <<" packets)";
  o<<"\nreceived packets: "<<DRS.completePackets;
  o<<"\nchecksum errors:  "<<DRS.totalCheckSumErrors;
  o<<"\nmissed syncs:     "<<DRS.totalMissedSyncs;
  o<<"\n------------------------------";
}


void printData(std::ostream &o, const CGIDataCartesian &data, const DDOptions &opt){
  if(opt.formatColumns){
    printPacketColumns(o, data);
    return;
  }
  if(opt.formatRealTimeCSV){
    printPacketCSV(o, data);
    return;
  }
  if(opt.formatHex) printPacketHex(o, data);
  else              printPacket   (o, data);
}


void printEndStatistics(std::ostream &o, const dataReceptionStatus &DRS, const DDOptions &opt){
  o<<std::dec<<"\n------------------------------";
  o<<std::dec<<"\n summary:                     ";
  o<<std::dec<<"\n------------------------------";
  printData(o, DRS.startDatum, opt);
  o<<std::dec<<"\n------------------------------";
  printData(o, DRS.endDatum, opt);
  o<<std::dec<<"\n------------------------------";
  o<<std::dec<<"\n------------------------------";
  o<<"\ntotal time:   "<<DRS.totalTime;
  o<<"\nframerate:    "<<DRS.completePackets / DRS.totalTime;
  o<<"\n------------------------------";
}

void getStatistics(const CGIDataCartesian &data, dataReceptionStatus &DRS, bool isFirst){
  struct CGIDataCartesianVersion1 *data1 = (struct CGIDataCartesianVersion1*)(void*)&data;
  if(isFirst){
    DRS.startTime = data1->time;
    DRS.startDatum = data;
  }
  /*
  if(isLast){
    DRS.endTime = data1->time;
    DRS.totalTime = DRS.endTime - DRS.startTime;
    cerr<<"end:"<<data1->time<<flush;
	}
  */
}


int main(int argc, char **argv){
  DDOptions opt;
  getOptions(argc, argv, opt);
  CGIDataCartesian data;
  class dataReceptionStatus DRS;
  const int bufSize = 300;
  char buf[bufSize];

#ifndef _MSC_VER
  int fd = open(opt.device.c_str(), O_RDONLY);
  if(fd == -1){
    cout<<"\n"<<strerror(errno)<<"!\ncannot open "<<opt.device<<"!\n";
    return(-1); 
  }
#else
  HANDLE fd = CreateFile(opt.device.c_str(),
						GENERIC_READ,
						0,
						0,
						OPEN_EXISTING,
						FILE_FLAG_OVERLAPPED,
						0 );
  if (fd == INVALID_HANDLE_VALUE)
  {
	  // error opening port; abort
	  cout<<"\n"<<GetLastError()<<"!\ncannot open "<<opt.device.c_str()<<"!\n";
		return(-1); 
  }
#endif

  bool isRegularFile = false;

#ifndef _MSC_VER
  struct stat stats;
  fstat(fd, &stats);
  if((stats.st_mode & S_IFMT) == S_IFREG){
    isRegularFile = true;
	}
#endif

  OVERLAPPED osReader = {0};
  setSerialAttributes(fd);

  while(1){
  
#ifndef _MSC_VER  
	  int charsRead = read(fd, buf, bufSize);
#else  
	  DWORD charsRead = 0;
	  if ( !ReadFile(fd, buf, bufSize, &charsRead, &osReader) ) {
		  if (GetLastError() != ERROR_IO_PENDING)
		  {
			  // error in communications; report it
			  break;
		  }
		  else
		  {
			  // waiting on read
			  Sleep(1000);
		  }
	  }
#endif
	  static int charCount = -1;
    //static int errorCount = -1;
    int fillState = CGI_DATA_LENGTH;
    //cout<<"chars read: "<<charsRead<<"\n";
    for(int i = 0; i < charsRead; i++){
      DRS.totalCharsRead++;
      charCount++;
      u8 input = buf[i];
      
      fillState = fillData(input, &data);
  
      if(fillState == -1){
        if(opt.verbose) cout<<"\nchecksum error!"<<flush;
        cerr<<"\nchecksum error!"<<flush;
        DRS.totalCheckSumErrors++;
        printData(cout, data, opt);
        printStatistics(cout, DRS);
        continue;              
      }

      if(fillState == CGI_DATA_LENGTH){
        getStatistics(data, DRS, (DRS.completePackets == 0));
        DRS.completePackets++;
        
        if(opt.printPackets)    printData(cout, data, opt);
        if(opt.printStatistics) printStatistics(cout, DRS);
        continue;           
      }

      if(DRS.totalCharsRead - fillState - DRS.nextSync > 0){
        if(opt.printUnsyncedChars){
          if((DRS.totalCharsRead - fillState - DRS.nextSync) % 8 == 1) cout<<endl;
          cout<<"0x"<<std::hex<<std::setw(2)<<(int)input<<"  ";
        }
        DRS.totalMissedSyncs++;
      }

      if(fillState == offsetOfFirstCGIDatum){
        //set nextSync to expected start of next sync-pattern
        DRS.nextSync = DRS.totalCharsRead + CGI_DATA_LENGTH - 4;
      }
      
    }
    if(!charsRead && isRegularFile) break;
  }
  struct CGIDataCartesianVersion1 *data1 = (struct CGIDataCartesianVersion1*)(void*)&data;
  DRS.endDatum = data;
  DRS.endTime = data1->time;
  DRS.totalTime = DRS.endTime - DRS.startTime;
  if(opt.summarize) printEndStatistics(cout, DRS, opt);
  cout<<"\n";
  return(0);
}



