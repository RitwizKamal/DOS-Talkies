#include<stdio.h>
#include<dos.h>
#include<stdlib.h>
#include<string.h>
#include<conio.h>

#define SW_INT 0x60

unsigned char dest_mac[]="\x08\x00\x27\x0a\xc3\x37";
unsigned char data[64];
unsigned char c[2];
int handle,packet_len,i;
unsigned char e[200];
unsigned char from_mac[6]; //source mac
unsigned char type[]="\xff\xff";

unsigned char broadcastAdd[]="\xff\xff\xff\xff\xff\xff";
unsigned char buffer[100];
int bufferCounter;

unsigned char temp;
int exitCounter=0;

void get_mac_address();
void fill_headers();
void fill_data(unsigned char *msg, int length);
void send_packet();
void getHandle();
void releaseType();

unsigned long int d,f;
unsigned char tt;
unsigned char temp_buffer[10];
unsigned long int fact(unsigned int n)
{
	if(n==0 || n==1)
		return 1;
	else
		return n*fact(n-1);
}
//-------------------------------------------
void interrupt receiver(bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,flags)
unsigned short bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,flags;
{
	if(ax==0)
	{
		if(cx>200)
		{
			es=0;
			di=0;
			return;
		}
		es=FP_SEG(e);
		di=FP_OFF(e);
		packet_len=cx;
	}
	if(ax==1)
	{
		if(memcmp(e,broadcastAdd,6)==0)
		{
			return;
		}
		for(i=6;i<12;i++)
		{
			cprintf("%02x:",e[i]);
		}
		cprintf("Received");
		for(i=14;i<packet_len;i++)
		{
			if(e[i]==0x00)
				break;
			putch(e[i]);
			tt=e[i];
		}
		
		d=tt-'0';
		f=fact(d);
		sprintf(temp_buffer,"%ld",f);
		fill_data(temp_buffer, strlen(temp_buffer));
		send_packet();
		putch('\r');
		putch('\n');
	}
}
//----------------------------------------------
//functions to be used
void getHandle()
{
	union REGS in,out;
	struct SREGS segregs;
	in.h.al=1;
	in.h.bx=-1;
	in.h.dl=0;
	in.h.cx=0;
	in.h.ah=2;
	segregs.es=FP_SEG(receiver);
	in.x.di=FP_OFF(receiver);
	in.x.si=FP_OFF(receiver);
	int86x(SW_INT,&in,&out,&segregs);
	handle=out.x.ax;
}

void releaseType()
{
	union REGS in,out;
	struct SREGS segregs;
	in.h.ah=3;
	in.x.bx=handle;
	int86x(SW_INT,&in,&out,&segregs);
}

void get_mac_address()
{
	union REGS in,out;
	struct SREGS segregs;
	char far *bufptr;
	segread(&segregs);
	bufptr = (char far *)from_mac;
	segregs.es = FP_SEG(bufptr);
	in.x.di = FP_OFF(from_mac);
	in.x.cx = 6;
	in.h.ah = 6;
	int86x(SW_INT, &in, &out, &segregs);
}

void fill_headers()
{
	memcpy(data, dest_mac, 6); 
	memcpy(data+6, from_mac, 6); 
	memcpy(data+12, type, 2); 
}

void fill_data(unsigned char *msg, int length)
{
	if(length+14 > 64)
	        length = 64-14;
	memcpy(data+14, msg, length);

	for(i=length+14; i<64; i++)
	{
		data[i] = 0;
	}
}

void send_packet()
{
	union REGS in,out;
	struct SREGS segregs;
	segread(&segregs);
	in.h.ah = 4;
	segregs.ds = FP_SEG(data);
	in.x.si = FP_OFF(data);
	in.x.cx = 64;
	int86x(SW_INT,&in,&out,&segregs);
}

//extra functions
void printReceiveMode()
{
	union REGS in,out;
	struct SREGS segregs;
	in.h.ah=21;
	in.x.bx=handle;
	int86x(SW_INT,&in,&out,&segregs);
	printf("Carry Flag Get Recieve Mode %d\n",out.x.cflag);
	printf("Get Recieve Mode %d\n",out.x.ax);
}

int setReceiveMode(int mode)
{
	union REGS in,out;
	struct SREGS segregs;
	in.h.ah=20;
	in.x.bx=handle;
	in.x.cx=mode;
	int86x(SW_INT,&in,&out,&segregs);
	printf("Carry Flag Set Recieve Mode %d\n",out.x.cflag);
	return out.x.cflag;
}
//extra functions end
//functions to be used end
//-----------------------------------------

int main()
{
        get_mac_address();
	//print source MAC address
	cprintf("My MAC address:-");
	for(i=0;i<6;i++)
	{
		printf("\x:",from_mac[i]);
	}
	//fill the Headers
	//i.e. dest_mac,source_mac,type
        fill_headers();
	getHandle();
        
        bufferCounter = 0;
        
        while(1)
        {
        	temp = getchar();
        	if(temp==0x0D || temp ==0x0A)
        	{
        		if(exitCounter >= 1)
        		{
        			break;
        		}
        		if(bufferCounter==0)
        			exitCounter++;
        		fill_data(buffer,bufferCounter);
        		bufferCounter = 0;
        		send_packet();
        	}
        	else
        	{
        		exitCounter = 0;
	        	buffer[bufferCounter] = temp;
	        	bufferCounter++;
	        }
        }

		releaseType();

		printf("Exiting\n");
		return 0;
}
//end----------------------------------------------------































