/*	AMIGADOS.C:	Operating specific I/O and Spawning functions
        for MicroEMACS 3.10
        (C)opyright 1988 by Daniel M. Lawrence
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "estruct.h"
#if    AMIGA
#include <exec/types.h>
#include <exec/io.h>
#include <intuition/intuition.h>
#include <devices/console.h>
#if !defined(__clang__)
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#else
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#endif
//#include "etype.h"
#include "edef.h"
#include "efunc.h"
//#include "elang.h"

#define CTRL    CONTROL        /* Control flag, or'ed in		*/
#define MOUS    0x1000        /* alternative input device (mouse)	*/
#define    SHFT    0x2000        /* shifted (for function keys)		*/
#define    ALTD    0x4000        /* ALT key...				*/

#define INTUITION_REV    0L
#define NEW            1006L

struct IntuitionBase *IntuitionBase;
struct Window *win;

struct IOStdReq con;        /* ptr to console device driver handle */

//#define USECUSTOMSCREEN
#ifdef USECUSTOMSCREEN
struct Screen *scr;

struct _palette {
  int r,g,b;
};

struct _palette palette[] = {
  0,0,0,
  205,0,0,
  0,205,0,
  205,205,0,
  0,0,238,
  205,0,205,
  0,205,205,
  229,229,229,

  127,127,127,
  255,0,0,
  0,255,0,
  255,255,0,
  92,92,255,
  255,0,255,
  0,255,255,
  255,255,255

};
#endif

/*	Intuition Function type declarations	*/

typedef struct {
  int rw_code;        /* normal keycode to generate */
  int rw_scode;        /* shifted  "  */
  int rw_ccode;        /* control  "  */
} RKEY;

/* raw keycode scan code to emacs keycode translation table */

RKEY keytrans[0x60] = {

    /*	CODE	NORM	SHIFT	CTRL */
    /*	0x00,*/    '`', '~', 0,
    /*	0x01,*/    '1', '!', 0,
    /*	0x02,*/    '2', '@', 0,
    /*	0x03,*/    '3', '#', 0,
    /*	0x04,*/    '4', '$', 0,
    /*	0x05,*/    '5', '%', 0,
    /*	0x06,*/    '6', '^', 0,
    /*	0x07,*/    '7', '&', 0,
    /*	0x08,*/    '8', '*', 0,
    /*	0x09,*/    '9', '(', 0,
    /*	0x0a,*/    '0', ')', 0,
    /*	0x0b,*/    '-', '_', 0,
    /*	0x0c,*/    '=', '+', 0,
    /*	0x0d,*/    '\\', '|', 0,
    /*	0x0e,*/    0, 0, 0,
    /*	0x0f,*/    '0', 0, 0,
    /*	0x10,*/    'q', 'Q', CTRL | 'Q',
    /*	0x11,*/    'w', 'W', CTRL | 'W',
    /*	0x12,*/    'e', 'E', CTRL | 'E',
    /*	0x13,*/    'r', 'R', CTRL | 'R',
    /*	0x14,*/    't', 'T', CTRL | 'T',
    /*	0x15,*/    'y', 'Y', CTRL | 'Y',
    /*	0x16,*/    'u', 'U', CTRL | 'U',
    /*	0x17,*/    'i', 'I', CTRL | 'I',
    /*	0x18,*/    'o', 'O', CTRL | 'O',
    /*	0x19,*/    'p', 'P', CTRL | 'P',
    /*	0x1a,*/    '[', '{', CTRL | '[',
    /*	0x1b,*/    ']', '}', 0,
    /*	0x1c,*/    0, 0, 0,
    /*	0x1d,*/    '1', SPEC | '>', 0,
    /*	0x1e,*/    '2', SPEC | 'N', 0,
    /*	0x1f,*/    '3', SPEC | 'V', 0,
    /*	0x20,*/    'a', 'A', CTRL | 'A',
    /*	0x21,*/    's', 'S', CTRL | 'S',
    /*	0x22,*/    'd', 'D', CTRL | 'D',
    /*	0x23,*/    'f', 'F', CTRL | 'F',
    /*	0x24,*/    'g', 'G', CTRL | 'G',
    /*	0x25,*/    'h', 'H', CTRL | 'H',
    /*	0x26,*/    'j', 'J', CTRL | 'J',
    /*	0x27,*/    'k', 'K', CTRL | 'K',
    /*	0x28,*/    'l', 'L', CTRL | 'L',
    /*	0x29,*/    ';', ':', 0,
    /*	0x2a,*/    39, 34, 0,
    /*	0x2b,*/    0, 0, 0,
    /*	0x2c,*/    0, 0, 0,
    /*	0x2d,*/    '4', SPEC | 'B', 0,
    /*	0x2e,*/    '5', 0, 0,
    /*	0x2f,*/    '6', SPEC | 'F', 0,
    /* this key is probably mapped on forign AIMIGA keyboards */
    /*	0x30,*/    0, 0, 0,
    /*	0x31,*/    'z', 'Z', CTRL | 'Z',
    /*	0x32,*/    'x', 'X', CTRL | 'X',
    /*	0x33,*/    'c', 'C', CTRL | 'C',
    /*	0x34,*/    'v', 'V', CTRL | 'V',
    /*	0x35,*/    'b', 'B', CTRL | 'B',
    /*	0x36,*/    'n', 'N', CTRL | 'N',
    /*	0x37,*/    'm', 'M', CTRL | 'M',
    /*	0x38,*/    ',', '<', 0,
    /*	0x39,*/    '.', '>', 0,
    /*	0x3a,*/    '/', '?', 0,
    /*	0x3b,*/    0, 0, 0,
    /*	0x3c,*/    '.', SPEC | 'D', 0,
    /*	0x3d,*/    '7', SPEC | '<', 0,
    /*	0x3e,*/    '8', SPEC | 'P', 0,
    /*	0x3f,*/    '9', SPEC | 'Z', 0,
    /*	0x40,*/    ' ', SHFT | ' ', 0,
    /*	0x41,*/    CTRL | 'H', SHFT | 'D', 0,
    /*	0x42,*/    CTRL | 'I', SHFT | 'I', 0,
    /*	0x43,*/    CTRL | 'M', CTRL | 'M', CTRL | 'M',
    /*	0x44,*/    CTRL | 'M', CTRL | 'M', CTRL | 'M',
    /*	0x45,*/    CTRL | '[', 0, 0,
    /*	0x46,*/    CTRL | 'D', 0, 0,
    /*	0x47,*/    0, 0, 0,
    /*	0x48,*/    0, 0, 0,
    /*	0x49,*/    0, 0, 0,
    /*	0x4a,*/    '-', 0, 0,
    /*	0x4b,*/    0, 0, 0,
    /*	0x4c,*/    CTRL | 'P', CTRL | 'Z', META | '<', /* cursor up    */
    /*	0x4d,*/    CTRL | 'N', CTRL | 'V', META | '>', /* cursor down  */
    /*	0x4e,*/    CTRL | 'F', CTRL | 'E', META | 'F', /* cursor right */
    /*	0x4f,*/    CTRL | 'B', CTRL | 'A', META | 'B', /* cursor left  */
    /*	0x50,*/    SPEC | '1', SHFT | SPEC | '1', CTRL | SPEC | '1',
    /*	0x51,*/    SPEC | '2', SHFT | SPEC | '2', CTRL | SPEC | '2',
    /*	0x52,*/    SPEC | '3', SHFT | SPEC | '3', CTRL | SPEC | '3',
    /*	0x53,*/    SPEC | '4', SHFT | SPEC | '4', CTRL | SPEC | '4',
    /*	0x54,*/    SPEC | '5', SHFT | SPEC | '5', CTRL | SPEC | '5',
    /*	0x55,*/    SPEC | '6', SHFT | SPEC | '6', CTRL | SPEC | '6',
    /*	0x56,*/    SPEC | '7', SHFT | SPEC | '7', CTRL | SPEC | '7',
    /*	0x57,*/    SPEC | '8', SHFT | SPEC | '8', CTRL | SPEC | '8',
    /*	0x58,*/    SPEC | '9', SHFT | SPEC | '9', CTRL | SPEC | '9',
    /*	0x59,*/    SPEC | '0', SHFT | SPEC | '0', CTRL | SPEC | '0',
    /*	0x5a,*/    '(', 0, 0,
    /*	0x5b,*/    ')', 0, 0,
    /*	0x5c,*/    '/', 0, 0,
    /*	0x5d,*/    '*', 0, 0,
    /*	0x5e,*/    '+', 0, 0,
    /*	0x5f,*/    META | '?', 0, 0,
};

/* some keyboard keys current states */

int r_shiftflag;    /* right shift key */
int l_shiftflag;    /* left shift key */
int r_altflag;        /* right alt key */
int l_altflag;        /* left alt key */
int r_amiflag;        /* right amiga key */
int l_amiflag;        /* left amiga key */
int ctrlflag;        /* control key */
int lockflag;        /* shift lock key */

/*	output buffers and pointers	*/

#define OBUFSIZE    1024L
#define    IBUFSIZE    64    /* this must be a power of 2 */

char out_buf[OBUFSIZE + 1];    /* output character buffer */
int out_ptr = 0;        /* index to next char to put in buffer */

/*	input buffers and pointers	*/

#define    IBUFSIZE    64    /* this must be a power of 2 */

unsigned char in_buf[IBUFSIZE];    /* input character buffer */
int in_next = 0;        /* pos to retrieve next input character */
int in_last = 0;        /* pos to place most recent input character */

void doevent(void);
void sendcon(char *buf);
void dokey(int c);
void stuffibuf(int key, int x, int y);
int mod(int c);

void in_init(void)    /* initialize the input buffer */

{
  in_next = in_last = 0;
}

int in_check(void)    /* is the input buffer non-empty? */

{
  if (in_next == in_last)
    return (FALSE);
  else
    return (TRUE);
}

void in_put(int event) /*int event - event to enter into the input buffer */
{
  in_buf[in_last++] = event;
  //printf("%d\n",in_last);
  in_last &= (IBUFSIZE - 1);
}

int in_get(void)    /* get an event from the input buffer */
{
  register int event;    /* event to return */

  event = in_buf[in_next++];
  in_next &= (IBUFSIZE - 1);
  return (event);
}

/*
 * This function is called once to set up the terminal device streams.
 * On VMS, it translates TT until it finds the terminal, then assigns
 * a channel to it and sets it raw. On CPM it is a no-op.
 */

#define DEPTH 1
#define NCOLORS (1<<DEPTH)

void ttopen(void) {
  struct NewWindow new_win;
#ifdef USECUSTOMSCREEN
  struct NewScreen new_scr;
#endif

  /* open the intuition library */
  IntuitionBase = (struct IntuitionBase *)
      OpenLibrary("intuition.library", INTUITION_REV);
  if (IntuitionBase == NULL) {
    printf("%%Can not open Intuition\n");
    exit(-1);
  }

  /* initialize the new windows attributes */
  new_win.LeftEdge = 0;
  new_win.TopEdge = 0;
  new_win.Width = 640;
  new_win.Height = 256;
  new_win.DetailPen = 0;
  new_win.BlockPen = 1;

#ifdef USECUSTOMSCREEN
  new_win.Title = NULL;
  new_win.Flags = WFLG_SMART_REFRESH | WFLG_ACTIVATE | WFLG_RMBTRAP | WFLG_NOCAREREFRESH | WFLG_BACKDROP | WFLG_BORDERLESS;
  //new_win.Flags = SMART_REFRESH | ACTIVATE | RMBTRAP | NOCAREREFRESH | BACKDROP | BORDERLESS;
  new_win.IDCMPFlags =  IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY;
  new_win.Type = CUSTOMSCREEN;
#else
  new_win.Title = (unsigned char *) "uEmacs 4.0.15";
#include <intuition/iobsolete.h>
//  new_win.Flags = WINDOWCLOSE | SMART_REFRESH | ACTIVATE |
//      WINDOWDRAG | WINDOWDEPTH | WINDOWSIZING | SIZEBRIGHT |
//      RMBTRAP | NOCAREREFRESH;
  new_win.Flags = WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_SIZEGADGET | WFLG_SIZEBRIGHT | WFLG_ACTIVATE
      | WFLG_RMBTRAP;
  new_win.IDCMPFlags = IDCMP_CLOSEWINDOW | IDCMP_NEWSIZE | IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY;
  //new_win.IDCMPFlags = CLOSEWINDOW | NEWSIZE | MOUSEBUTTONS | RAWKEY;
  new_win.Type = WBENCHSCREEN;
#endif
  new_win.FirstGadget = NULL;
  new_win.CheckMark = NULL;
  new_win.Screen = NULL;
  new_win.BitMap = NULL;
  new_win.MinWidth = 100;
  new_win.MinHeight = 25;
  new_win.MaxWidth = 640;
  new_win.MaxHeight = 256;

#ifdef USECUSTOMSCREEN
  int coltran[16] = {2, 3, 5, 7, 0, 4, 6, 1,8, 12, 10, 14, 9, 13, 11, 15};
  //extern int coltran[];
  memset(&new_scr,0,sizeof(struct NewScreen));
  new_scr.DetailPen = coltran[gfcolor];//0;
  new_scr.BlockPen = coltran[gbcolor];//1;
  new_scr.LeftEdge = 0;
  new_scr.TopEdge = 0;
  new_scr.Width = 640;
  new_scr.Height = 256;
  new_scr.Depth = DEPTH;
  new_scr.Type = CUSTOMSCREEN;
  new_scr.ViewModes = HIRES;
  new_scr.DefaultTitle = "uEmacs 4.0.15";

  //printf("Going to open screen...\n");
  scr = (struct Screen *)OpenScreen(&new_scr);
  if (scr == NULL) {
    printf("%%Can not open screen\n");
    exit(-2);
  }

  int i;

  for(i=0; i<NCOLORS; ++i)
    SetRGB4(&scr->ViewPort,coltran[i],palette[i].r>>4,palette[i].g>>4,palette[i].b>>4 );
  new_win.Screen = scr;
  //new_win.TopEdge = scr->BarHeight-scr->Font->ta_YSize;
  int t=scr->BarHeight+1;
  new_win.TopEdge = t;

  //printf("t: %d\n",t);
  new_win.Height = scr->Height-t;
#endif

  /* open the window! */
  win = (struct Window *) OpenWindow(&new_win);
  if (win == NULL) {
#ifdef USECUSTOMSCREEN
    CloseScreen(scr);
#endif
    printf("%%Can not open a window\n");
    exit(-2);
  }

  //printf("%d %d\n", (int)win->BorderLeft, (int)win->BorderRight);
  /* and open up the console for output */
  con.io_Data = (APTR) win;
  OpenDevice("console.device", 0, (struct IORequest *) &con, 0);

  /* and init all the keyboard flags */
  r_shiftflag = FALSE;
  l_shiftflag = FALSE;
  r_altflag = FALSE;
  l_altflag = FALSE;
  r_amiflag = FALSE;
  l_amiflag = FALSE;
  ctrlflag = FALSE;
  lockflag = FALSE;

  /* initialize our private event queue */
  in_init();

  /* set the current sizes */

  newwidth(TRUE, (win->Width - (win->BorderLeft + win->BorderRight)) / win->IFont->tf_XSize);
  printf("W: %d LeftB: %d RightB: %d [%d]\n", win->Width, win->BorderLeft, win->BorderRight, (win->Width-(win->BorderLeft+win->BorderRight))/8);
#define AMIGAMAXLINES 28
  int newh = (win->Height - (win->BorderTop + win->BorderBottom)) / win->IFont->tf_YSize;
  if (newh>AMIGAMAXLINES)
    newh = AMIGAMAXLINES;
  newsize(TRUE, newh); // alkis - was 23
  //printf("alkis -> %d\n",(win->Height-(win->BorderTop+win->BorderBottom))/8);
  //newsize(TRUE, 26); // alkis - was 23

  /* on all screens we are not sure of the initial position
     of the cursor					*/
  ttrow = 999;
  ttcol = 999;
}

/*
 * This function gets called just before we go back home to the command
 * interpreter. On VMS it puts the terminal back in a reasonable state.
 * Another no-operation on CPM.
 */
void ttclose(void) {
  /* make sure there is no pending output */
  ttflush();

  /* and now close up shop */
  CloseDevice((struct IORequest *) &con);
  CloseWindow(win);
#ifdef USECUSTOMSCREEN
  CloseScreen(scr);
#endif
  OpenWorkBench();
}

/*
 * Write a character to the display. On VMS, terminal output is buffered, and
 * we just put the characters in the big array, after checking for overflow.
 * On CPM terminal I/O unbuffered, so we just write the byte out. Ditto on
 * MS-DOS (use the very very raw console output routine).
 */

int ttputc(int c) {
  /* add the character to the output buffer */
  out_buf[out_ptr++] = c;

  /* send the buffer out if we are at the limit */
  if (out_ptr >= OBUFSIZE)
    ttflush();
}

/*
 * Flush terminal buffer. Does real work where the terminal output is buffered
 * up. A no-operation on systems where byte at a time terminal I/O is done.
 */

void ttflush(void) {
  /* if there are any characters waiting to display... */
  if (out_ptr) {
    out_buf[out_ptr] = 0; /* terminate the buffer string */
    sendcon(out_buf); /* send them out */
    out_ptr = 0; /* and reset the buffer */
  }
}

/*
 * Read a character from the terminal.
 */
int ttgetc(void) {
  while (1) {
    /* make sure there is no pending output */
    ttflush();

    /* if it is already buffered up, get it */
    if (in_check())
      break;

    /* process an INTUITION event (possibly loading the input buffer) */
    doevent();
  }
  return (in_get());
}
//int ttgetc(void)
//{
//	/* make sure there is no pending output */
//	nxtchr: ttflush();
//
//	/* if it is already buffered up, get it */
//	if (in_check())
//		return (in_get());
//
//	/* process an INTUITION event (possibly loading the input buffer) */
//	doevent();
//	goto nxtchr;
//}



#if    TYPEAH
/* typahead:	Check to see if any characters are already in the
   keyboard buffer
*/

int typahead(void) {

  /* if type ahead is already pending... */
  if (in_check())
    return (TRUE);

  /* check the signal for IDCMP events pending */
  //if ((1 << win->UserPort->mp_SigBit) != 0)
  //if (GetMsg(win->UserPort))
  //return(TRUE);

  /* no event in queue... no typeahead ready */
  return (FALSE);
}
#endif

void doevent(void) {
  register int eventX, eventY;    /* local copies of the event info */
  struct IntuiMessage *event;    /* current event to repond to */
  ULONG class;    /* class of event */
  USHORT code;    /* data code */
  SHORT x, y;    /* mouse x/y position at time of event */


  /* wait for an event to occur */
  Wait(1 << win->UserPort->mp_SigBit);

  //printf("Waiting ended. ");
  /* get the event and parse it up */
  while ((event = (struct IntuiMessage *) GetMsg(win->UserPort))) {
    class = event->Class;
    code = event->Code;
    eventX = event->MouseX;
    eventY = event->MouseY;
    ReplyMsg((struct Message *) event);

    /* a normal keystroke? */
    if (class == IDCMP_RAWKEY) {
      //static int alkc=0;
      //printf("key #%d\n",++alkc);
      dokey(code);
      continue;
    }

    /* User clicked on the close gadget! */
    if (class == IDCMP_CLOSEWINDOW) {
      quit(FALSE, 0);
      stuffibuf(255, 0, 0);    /* fake a char to force quit to work */
    }

    /* resolve the mouse address (border adjusted) */
    if (class == IDCMP_NEWSIZE) {
      x = (win->Width - (win->BorderLeft + win->BorderRight)) / win->IFont->tf_XSize;
      y = (win->Height - (win->BorderTop + win->BorderBottom)) / win->IFont->tf_YSize;
      newwidth(TRUE, x);
      newsize(TRUE, y);
      sgarbf = TRUE;
      stuffibuf(CTRL | 'F', 0, 0);
      //redraw(TRUE,0);
    } else {
      x = (eventX - win->BorderLeft) / win->IFont->tf_XSize;
      y = (eventY - win->BorderTop) / win->IFont->tf_YSize;
    }
    //printf("code: %d x:%d y:%d\n", code, x, y);

    /*
    if (x > 77)
      x = 77;
    if (y > AMIGAMAXLINES)
      y = AMIGAMAXLINES;
    */

    /* are we resizing the window?
    if (class == NEWSIZE) {
      stuffibuf(MOUS | '1', x, y);
      continue;
    }
    */

#ifdef MOUSEWORKS
    // and lastly, a mouse button press
    switch (code) {
      case 104:	stuffibuf(MOUS | mod('a'), x, y);
        break;
      case 232:	stuffibuf(MOUS | mod('b'), x, y);
        break;
      case 105:	stuffibuf(MOUS | mod('e'), x, y);
        break;
      case 233:	stuffibuf(MOUS | mod('f'), x, y);
        break;
    }
#endif

  }
}

int mod(int c)    /* modify a character by the current shift and control flags */
{
  /* first apply the shift and control modifiers */
  if (l_shiftflag || r_shiftflag || lockflag)
    c -= 32;
  if (ctrlflag)
    c |= CTRL;
  return (c);
}

void sendcon(char *buf)    /* send a string to the console */
/*char *buf;	 buffer to write out */
{
  /* initialize the IO request */
  con.io_Data = (APTR) buf;
  con.io_Length = strlen(buf);
  con.io_Command = CMD_WRITE;

  /* and perform the I/O */
  SendIO((struct IORequest *) &con);
}

/* process an incomming keyboard code */
void dokey(int code)
/* int code;	aw keycode to convert */
{
  register int ekey;    /* translate emacs key */
  register int dir;    /* key direction (up/down) */

  //printf("%2X ok\n",code);
  /* decode the direction of the key */
  dir = TRUE;
  if (code > 127) {
    code = code & 127;
    dir = FALSE;
  }

  /* process various shift keys */
  if (code >= 0x60) {
    switch (code) {

      case 0x60: l_shiftflag = dir;
        break;
      case 0x61: r_shiftflag = dir;
        break;
      case 0x62: lockflag = dir;
        break;
      case 0x63: ctrlflag = dir;
        break;
      case 0x64: l_altflag = dir;
        break;
      case 0x65: r_altflag = dir;
        break;
      case 0x66: l_amiflag = dir;
        break;
      case 0x67: r_amiflag = dir;
        break;

    }
    return;
  }
  //printf("%d%d%d%d%d%d%d%d\n",l_shiftflag,r_shiftflag,lockflag,ctrlflag,l_altflag,r_altflag,l_amiflag,r_amiflag);
  /* up keystrokes are ignored for the rest of these */
  if (dir == FALSE)
    return;

  /* first apply the shift and control modifiers */
  if (ctrlflag)
    ekey = keytrans[code].rw_ccode;
  else if (l_shiftflag || r_shiftflag || lockflag)
    ekey = keytrans[code].rw_scode;
  else
    ekey = keytrans[code].rw_code;

  /* now apply the ALTD modifier */
  //if (r_altflag || l_altflag)
  if (r_amiflag || l_amiflag)
    ekey |= ALTD;

  /* apply the META prefix */
  if (r_altflag || l_altflag || (ekey & META)) {
    stuffibuf(CTRL | '[', 0, 0);
    if (ekey & META) ekey &= ~META;
    if ('a' <= ekey && ekey <= 'z') {
      ekey -= 32;
      //printf("%c\n",ekey);
      //ekey |= META;
    }
//    else {
//      printf("not in range %c (%X)\n",ekey,ekey);
//    }
  }

  /* and place it in the input buffer */
  stuffibuf(ekey, 0, 0);
}

void stuffibuf(int key, int x, int y)    /* stuff a key in the input buffer */
{
  int upper;    /* upper extended bits of key */

  /* split the extended keystroke */
  upper = key >> 24;
  key = key & 255;

  /* if it is JUST control... encode it in! */
  if (upper == (CTRL >> 24)) {
    in_put(key - 64);
    return;
  }

  /* if it is normal, just place it inqueue */
  if (upper == 0) {
    in_put(key);
    return;
  }

  /* queue up an extended escape sequence */
  in_put(0);        /* escape indicator */
  in_put(upper);        /* event type */
  //if (upper & (MOUS >> 24)) {
  if (upper & MOUS) {
    in_put(x);    /* x position */
    in_put(y);    /* y position */
    return;
  }
  in_put(key);        /* event code */
}

/*
 * Create a subjob with a copy of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C". The message at the start in VMS puts out a newline.
 * Under some (unknown) condition, you don't get one free when DCL starts up.
 */
int spawncli(int f, int n) {

  /* don't allow this command if restricted */
  if (restflag)
    return (resterr());

  mlwrite("[Starting new CLI]");
  /*              "[Starting new CLI]" */
  sgarbf = TRUE;
  Execute("NEWCLI \"CON:0/0/640/200/MicroEMACS Subprocess\"", 0L, 0L);
  return (TRUE);
}

/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X !".
 */
int spawn(int f, int n) {
  register int s;
  char line[NLINE];

  long newcli;

  /* don't allow this command if restricted */
  if (restflag)
    return (resterr());

  if ((s = mlreply("!", line, NLINE)) != TRUE)
    return (s);
  newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
  Execute(line, 0L, newcli);
  Close(newcli);
  tgetc(); /* Pause.               */
  sgarbf = TRUE;
  return (TRUE);
}

/*
 * Run an external program with arguments. When it returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X $".
 */

int execprg(int f, int n) {
  register int s;
  char line[NLINE];

  long newcli;

  /* don't allow this command if restricted */
  if (restflag)
    return (resterr());

  if ((s = mlreply("!", line, NLINE)) != TRUE)
    return (s);
  newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
  Execute(line, 0L, newcli);
  Close(newcli);
  tgetc(); /* Pause.               */
  sgarbf = TRUE;
  return (TRUE);
}

/*
 * Pipe a one line command into a window
 * Bound to ^X @
 */
int pipecmd(int f, int n) {
  register int s;    /* return status from CLI */
  register struct window *wp;    /* pointer to new window */
  register struct buffer *bp;    /* pointer to buffer to zot */
  char line[NLINE];    /* command line send to shell */
  static char bname[] = "command";

  static char filnam[] = "ram:command";
  long newcli;

  /* don't allow this command if restricted */
  if (restflag)
    return (resterr());

  /* get the command to pipe in */
  if ((s = mlreply("@", line, NLINE)) != TRUE)
    return (s);

  /* get rid of the command output buffer if it exists */
  if ((bp = bfind(bname, FALSE, 0)) != FALSE) {
    /* try to make sure we are off screen */
    wp = wheadp;
    while (wp != NULL) {
      if (wp->w_bufp == bp) {
        onlywind(FALSE, 1);
        break;
      }
      wp = wp->w_wndp;
    }
    if (zotbuf(bp) != TRUE)

      return (FALSE);
  }

  newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
  strcat(line, " >");
  strcat(line, filnam);
  Execute(line, 0L, newcli);
  s = TRUE;
  Close(newcli);
  sgarbf = TRUE;

  if (s != TRUE)
    return (s);

  /* split the current window to make room for the command output */
  if (splitwind(FALSE, 1) == FALSE)
    return (FALSE);

  /* and read the stuff in */
  if (getfile(filnam, FALSE) == FALSE)
    return (FALSE);

  /* make this window in VIEW mode, update all mode lines */
  curwp->w_bufp->b_mode |= MDVIEW;
  wp = wheadp;
  while (wp != NULL) {
    wp->w_flag |= WFMODE;
    wp = wp->w_wndp;
  }

  /* and get rid of the temporary file */
  DeleteFile(filnam);
  return (TRUE);
}

/*
 * filter a buffer through an external DOS program
 * Bound to ^X #
 */
int filter(int f, int n) {
  register int s;    /* return status from CLI */
  register struct buffer *bp;    /* pointer to buffer to zot */
  char line[NLINE];    /* command line send to shell */
  char tmpnam[NFILEN];    /* place to store real file name */
  static char bname1[] = "fltinp";

  static char filnam1[] = "ram:fltinp";
  static char filnam2[] = "ram:fltout";
  long newcli;

  /* don't allow this command if restricted */
  if (restflag)
    return (resterr());

  if (curbp->b_mode & MDVIEW)    /* don't allow this command if	*/
    return (rdonly());    /* we are in read only mode	*/

  /* get the filter name and its args */
  if ((s = mlreply("#", line, NLINE)) != TRUE)
    return (s);

  /* setup the proper file names */
  bp = curbp;
  strcpy(tmpnam, bp->b_fname);    /* save the original name */
  strcpy(bp->b_fname, bname1);    /* set it to our new one */

  /* write it out, checking for errors */
  if (writeout(filnam1) != TRUE) {
    mlwrite("[Cannot write filter file]");
    /*                      "[Cannot write filter file]" */
    strcpy(bp->b_fname, tmpnam);
    return (FALSE);
  }

  newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
  strcat(line, " <ram:fltinp >ram:fltout");
  Execute(line, 0L, newcli);
  s = TRUE;
  Close(newcli);
  sgarbf = TRUE;

  /* on failure, escape gracefully */
  if (s != TRUE || (readin(filnam2, FALSE) == FALSE)) {
    mlwrite("[Execution failed]");
    /*                      "[Execution failed]" */
    strcpy(bp->b_fname, tmpnam);
    DeleteFile(filnam1);
    DeleteFile(filnam2);
    return (s);
  }

  /* reset file name */
  strcpy(bp->b_fname, tmpnam);    /* restore name */
  bp->b_flag |= BFCHG;        /* flag it as changed */

  /* and get rid of the temporary file */
  DeleteFile(filnam1);
  DeleteFile(filnam2);
  return (TRUE);
}

/* return a system dependant string with the current time */
char *timeset(void) {
  return "Error";
}

#if    COMPLET && AZTEC
/*	FILE Directory routines		*/

char path[NFILEN];	/* path of file to find */
char rbuf[NFILEN];	/* return file buffer */


/*	do a wild card directory search (for file name completion) */

char *getffile(fspec)

    char *fspec;	/* pattern to match */

{
  register int index;		/* index into various strings */
  char fname[NFILEN];		/* file/path for DOS call */

  /* first parse the file path off the file spec */
  strcpy(path, fspec);
  index = strlen(path) - 1;
  while (index >= 0 && (path[index] != '/' &&
                        path[index] != '\\' && path[index] != ':'))
    --index;
  path[index+1] = 0;

  /* construct the composite wild card spec */
  strcpy(fname, path);
  strcat(fname, &fspec[index+1]);
  strcat(fname, "*.*");

  /* save the path/wildcard off */
  strcpy(path, fname);

  /* and call for the first file */
  return(getnfile());
}

char *getnfile()
{
  register char *sp;	/* return from scdir */

  /* and call for the next file */
  //sp = scdir(path);
  sp = NULL;
  fprintf(stderr, "I have to implement scdir -- amigados.c\n");
  if (sp == NULL)
    return(NULL);

  /* return the next file name! */
  strcpy(rbuf, sp);
  return(rbuf);
}
#else

#include <dos/dos.h>

__aligned struct FileInfoBlock fib;
static BPTR fc_lock = 0;   /* File completion lock */
static char fullfname[108];

/*
 * returns the non-filename prefix
 * ie:  df0:foo -> returns df0:
 *              foo/boo -> returns foo/
 *              df0:foo/boo -> returns df0:foo/
 */
char *getDirectory(const char *text) {
  static char buffer[80];
  int lastplace = -1;

  int i;
  for (i = 0; text[i]; ++i) {
    buffer[i] = text[i];
    if (text[i] == ':' || text[i] == '/')
      lastplace = i;
  }
  if (lastplace == -1)
    return "";
  buffer[lastplace + 1] = '\0';
  return buffer;
}

char *getnext(char *fspec);

char *getfirst(char *fspec) {
  char *base = getDirectory(fspec);
  char *starts = fspec + strlen(base);

  if (fc_lock)
    UnLock(fc_lock);

  fc_lock = Lock((STRPTR) base, ACCESS_READ);
  if (fc_lock) {
    if (Examine(fc_lock, &fib)) {
      if (!strncasecmp(starts, fib.fib_FileName, strlen(starts))) {
        //printf("starts: %s filename: %s\n", starts, fib.fib_FileName);
        strcpy(fullfname, base);
        strcat(fullfname, fib.fib_FileName);
        return fullfname;
      }
      return getnext(fspec);
    } else {
      //printf("Examine() failed\n");
      UnLock(fc_lock);
      fc_lock = 0;
      return NULL;
    }
  } else {
    printf("Can't obtain lock on \"%s\"\n", fspec);
    return NULL;
  }
}

char *getnext(char *fspec) {
  char *base = getDirectory(fspec);
  char *starts = fspec + strlen(base);
  BOOL ret;
  LONG error;

  if (fc_lock == 0)
    return NULL;
  do {
    ret = (BOOL) ExNext(fc_lock, &fib);
  } while (ret != FALSE && strncasecmp(starts, fib.fib_FileName, strlen(starts)));
  if (ret == FALSE) {
    error = IoErr();
    if (error == ERROR_NO_MORE_ENTRIES) {
      /* yeah, we are done. Shut it down.. */
      UnLock(fc_lock);
      fc_lock = 0;
      // printf("we run out of entries...\n");
      return NULL;
    }
    printf("Something went wrong: %ld (amiga IoErr())\n", error);
    return NULL;
  }
  // printf("starts: %s filename: %s\n", starts, fib.fib_FileName);
  strcpy(fullfname, base);
  strcat(fullfname, fib.fib_FileName);
  return fullfname;;
}

void closeFileCompletionLock(void) {
  if (fc_lock)
    UnLock(fc_lock);
}

#endif
#else
void adoshello()
{
}
#endif
