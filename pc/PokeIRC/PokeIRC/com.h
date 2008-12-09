#ifndef _COM_H_
#define _COM_H_

/******************************************************************************
******************************************************************************/
#define COM_AUTO	(0xFF)
#define COM_PORT4	(3)
#define COM_PORT5	(4)
#define COM_PORT12	(11)
/******************************************************************************
******************************************************************************/
bool ComIsConnect(void);
TCHAR *ComGetPort(void);
/******************************************************************************
******************************************************************************/
bool ComInit(unsigned char idx); //bool ComInit(int idx);
void ComEnd();
bool WriteData(unsigned char *buff, unsigned int data_size);
int ReadData(unsigned char *buff, unsigned int max_size);

bool ComCheck(void);

#endif
