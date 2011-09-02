/*
 * qzone_protocol.h : QzoneЭ��ͳһ�ӿ�
 * 
 *  Copyright (c) Tencent 2007.
 * 
 * History: 
 * 0.06  6--2010, microsun modifyed
    Note:   1.Ԥ���ֶ�����,���Ӻ���SetReserved()
 * 0.04, 9 -- 2008, geminiguan modified
 	Note:	1.����AppendLongLong��FetchLongLong������������ͽ��longlong������
 * 0.09, 10--2007, ianyang modified
 	Note:	1. ����m_iErrno��Ա������¼���/����Ƿ����, ���ṩ��Ӧ�ӿ�
 * 0.07, 31-may-2007, ianyang modified
 	Note:	1. ����AppendData() & FetchData() ����unsigned ����֧��, ����ǿ��ת��
 			2. [FIXBUG] �޸�AppendData() & FetchData() ��DataLenΪunsigned
 * 0.05, 28-may-2007, ianyang modified
	Note:	1. ��������packet�ӿ�, �ṩȫ�ں�ʽ�ӿ�	
			2. �ṩMapPacketBuffer() & UnmapPacketBuffer()��Ϊӳ�����ݰ�buffer�Ľӿ�
			3. �ṩӳ��Э��ͷbuffer��ȡЭ��ͷ��Ϣ�Ľӿ�
			4. �޸�FetchData(), ����memcpy��������
			5. �޸�Input() & Output(), �������ظ����÷���
			6. ����QzoneServerResponse����QzoneClient��ʾЭ��Ϊ�ͻ�������, ����Э������
 * 0.03, 16-may-2007, ianyang modified
	Note: 	1. ȥ��client��Ϣ, ����Ⱦɫ��Ϣ
 			2. ȥ���ְ���Ϣ, ��ϵͳ�����Ա��֤��ʹ��UDP��ʹ�÷ְ����
 			3. ���ǵ�64λ��32λ��������, �޸�����ulongΪuint
 			4. ȥ���ض�����Ϣ, �Ƿ��ض���ϲ���serverResponseFlag, ������ض���ip,port�����Э������
 			5. ��������CQzonePacket�ӿ�
 * 0.01, 15-may-2007, ianyang create
 * 
 */

#ifndef _QZONE_PROTOCOL_H_
#define _QZONE_PROTOCOL_H_

#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string>

////add by geminiguan
#include <sys/types.h>
#include <endian.h>
#include <byteswap.h>
#include <netinet/in.h>


#if BYTE_ORDER == LITTLE_ENDIAN
#define htonq(x) bswap_64(x)
#elif BYTE_ORDER == BIG_ENDIAN
#define htonq(x)  (x)
#endif
///end add by geminiguan

using namespace std;

// -----------------------------------------------------------------------------
//
// server response ����
//
// -----------------------------------------------------------------------------
typedef enum
{
	QzoneServerSucc = 0,	// 0 - [��������, ����ɹ�]
	QzoneServerFailed = 1,	// 1 - [��������, ����ʧ��]
	QzoneServerExc = 2,		// 2 - [�쳣����, �������ܾ�����]
	QzoneServerBusy = 3,	// 3 - [��������, ������æ, ������]	
	QzoneServerRedirected = 10,	// 10 - [�������ض���]
	QzoneServerAck = 20,	// 20 - [��ִ��]
	QzoneClient = 100,		// 100 - [client����, ��server��Ӧ]
}QzoneServerResponse;

#define DefaultServResInfo 0



/*
 * protocol head
 ------------------------------------------------------------------------------------------------------------
| �汾(1byte) | ������(4 bytes) | Ч���(2 bytes) | ���к�(4 bytes) | ���к�(4 bytes) | Ⱦɫ��Ϣ(4 bytes) | 
 ------------------------------------------------------------------------------------------------------------
| server��Ӧ��ʶ(1 byte) | server��Ӧ��Ϣ(2 bytes) | Э���ܳ���(4bytes) | Э����|
----------------------------------------------------------------------------------------
 */ 
#pragma pack(1)
typedef struct _QzoneProtocolHead_
{
	unsigned char version;
	unsigned int cmd;
	unsigned short checksum;
	
	unsigned int serialNo;				// 4 bytes, Protocol Serial Number, ��client����,clientЧ��	
	unsigned int colorration;			// 4 bytes, Ⱦɫ��Ϣ	
	unsigned char serverResponseFlag;	// 1 byte, Server�˻�Ӧ��ʶ : 
										/* 	0 - [��������, ����ɹ�], 
											1 - [��������, ����ʧ��]
											2 - [�쳣����, �������ܾ�����]
											3 - [��������, ������æ, ������]											
											10 - [�������ض���]
											20 - [��ִ��],
											100 - [client����, ��server��Ӧ]
										*/
	unsigned short serverResponseInfo;	// 2 bytes, Server�˻�Ӧ������Ϣ 
										/*	���ڴ���ʧ��(1):  ��ʾ����ʧ�ܵĴ����errcode
											���ڷ�����æ(3):  ��ʾ����ʱ��(�����ֽ���)
											���ڷ������ܾ�����(2): ��ʾ�ܾ�ԭ��(�����ֽ���)
											����, �������ܾ�����ԭ��������:
											ʹ�õ�ÿbit��ʾ��ͬ�ľܾ�����, �ɵ�λ�ֽ����߷ֱ���Ϊ:		
												0x1: ��ǰЭ��汾
												0x2: ��ǰЭ��������
												0x4: ��ǰclient����
												0x8: ��ǰclient�汾
												0x10: ��ǰclient��ϵͳ
												��Ӧ��λ��1��ʾ�ܾ�, ��0��ʾ���ܾ�, ��5λȫΪ0��ʾ�����ɾܾ�.
											����, �������ܾ���ǰclient���͵ĵ�ǰclient�汾, ��ServerResponseInfo��ȡֵΪ0x12.
										*/
	char reserved[1];					// Ԥ��
	
	unsigned int len;					// Э���ܳ���
	
	_QzoneProtocolHead_()
	{
		version = 0x0;
		cmd = 0;
		checksum = 0;
		serialNo = 0;
		colorration = 0;
		serverResponseFlag = 0;
		serverResponseInfo = 0;
		len = 0;
		reserved[0] = 0;
	}

	void Encode()
	{
		version = version;
		cmd = htonl(cmd);		
		serialNo = htonl(serialNo);
		colorration = htonl(colorration);
		serverResponseFlag = serverResponseFlag;
		serverResponseInfo = htons(serverResponseInfo);
		len = htonl(len);

		return ;
	}
	void Decode()
	{
		version = version;
		cmd = ntohl(cmd);		
		serialNo = ntohl(serialNo);
		colorration = ntohl(colorration);
		serverResponseFlag = serverResponseFlag;
		serverResponseInfo = ntohs(serverResponseInfo);
		len = ntohl(len);

		return ;
	}

	inline void operator = (_QzoneProtocolHead_ &head)
	{
		version = head.version;
		cmd = head.cmd;
		serialNo = head.serialNo;
		colorration = head.colorration;
		serverResponseFlag = head.serverResponseFlag;
		serverResponseInfo = head.serverResponseInfo;
		len = head.len;

		return ;
	}

	/*
	@brief:	����Э��ͷ������Ϣ
	@param:	�汾, ������
	*/
	inline void SetHead(unsigned char version_, unsigned int cmd_)
	{
		version = version_;
		cmd = cmd_;
		return ;
	}

	/*
	@brief:	����Э��ͷ���к�
	@param:	���к�
	*/
	inline void SetSerialNo(unsigned int serialNo_)
	{
		serialNo = serialNo_;
		return ;
	}	

	/*
	@brief:	����Ⱦɫ��Ϣ
	@param:	Ⱦɫ��Ϣ
	*/
	inline void SetColorration(unsigned int colorration_)
	{
		colorration = colorration_;
		return ;
	}

	/*
	@brief:	Server����Ӧ����
	@param:	��Ӧ��ʶ, ��Ӧ��Ϣ
	*/
	inline void SetServerResponse(QzoneServerResponse serverResponseFlag_, unsigned short serverResponseInfo_)
	{
		serverResponseFlag = serverResponseFlag_;
		serverResponseInfo = serverResponseInfo_;		
		return ;
	}

	/*
	@brief:	����Э�鳤��
	@param:	Э�鳤��(���ĳ���)
	*/
	inline void SetLen(unsigned int len_)
	{
		len = len_;
		return ;
	}
	
	/*
	@brief:	����Ԥ���ֶ�
	@param:	Ԥ���ֶ�����
	*/	
	inline void SetReserved(unsigned char reserved_)
	{
		reserved[0] = reserved_;
		return ;
	}	

	/*
	@brief:	Ч���
	@param:	Э��ͷ+Э�����sendbuf, sendbuf����
	*/
	inline unsigned short CheckSum(const void* buf, unsigned int bufLen)
	{
		unsigned int sum = 0;
		unsigned short* data = (unsigned short*) buf;
		
		int len = bufLen / 2;
		int mod = bufLen % 2;
		for(int i = 0;i < len; i++)
			sum += data[i];
		
		unsigned short nshort = 0;
		if (mod == 1)
		{
			/* bugfix, 2008-05-28, ianyang modified, char* => unsigned char* */
			nshort = (unsigned short) ((unsigned char*) buf)[bufLen - 1];
			sum += nshort;
		}		
		sum = (sum >> 16) + (sum & 0xffff);
		sum += (sum >> 16);
		nshort = ~((unsigned short)sum);		
		
		return nshort;		
	}
} QzoneProtocolHead, *QzoneProtocolHeadPtr;
#pragma pack()

// -----------------------------------------------------------------------------
//
// Э��ṹ
//
// -----------------------------------------------------------------------------

/*
 * ���ݰ���ͷβ��ʶ
 */
#define QzoneProtocolSOH			0x04
#define QzoneProtocolEOT			0x05

/*
 *  protocol packet
 */
#pragma pack(1)

typedef struct 
{
	char soh;
	QzoneProtocolHead head;
	char body[];
	// ... char eot; ������
}QzoneProtocol, *QzoneProtocolPtr;

#pragma pack()



// -----------------------------------------------------------------------------
//
// qzone protocol packet; �ṩ����CQZonePackage�Ľӿ�
//
// -----------------------------------------------------------------------------
class QzoneProtocolPacket
{
public:
	QzoneProtocolPtr m_pPacket;
	
private:	
	unsigned int m_iPos;
	unsigned int m_iTail;
	unsigned int m_iBodyMaxLen;

	bool m_bInput;
	bool m_bOutput;

	bool m_bCreated;
    bool m_bNew;

	int m_iErrno;
	
public:
	QzoneProtocolPacket()
		:m_pPacket(NULL),
		m_iPos(0),
		m_iTail(0),
		m_iBodyMaxLen(0),
		m_bInput(false),
		m_bOutput(false),
		m_bCreated(false),
		m_bNew(false),
		m_iErrno(0)
	{
	}
	~QzoneProtocolPacket()
	{
		FreePacket();
	}
	

/// ����Э��ͷ��Ϣ�Ľӿ�
public:
	inline void SetHead(unsigned char version_, unsigned int cmd_)
	{		
		m_pPacket->head.SetHead(version_, cmd_);
		return ;
	}
	inline void SetSerialNo(unsigned int serialNo_)
	{
		m_pPacket->head.SetSerialNo(serialNo_);
		return ;
	}
	inline void SetColorration(unsigned int colorration_)
	{
		m_pPacket->head.SetColorration(colorration_);
		return ;
	}
	inline void SetServerResponse(QzoneServerResponse serverResponseFlag, unsigned short serverResponseInfo)
	{
		m_pPacket->head.SetServerResponse(serverResponseFlag, serverResponseInfo);
		return ;
	}
	inline void SetLen(unsigned int len_)
	{
		m_pPacket->head.SetLen(len_);
		return ;
	}
	inline void SetHead(QzoneProtocolHead &head_)
	{		
		memcpy((void*) &(m_pPacket->head), (void*) &head_, sizeof(QzoneProtocolHead));
		return ;
	}

	inline void SetReserved(unsigned char reserved)
	{
		m_pPacket->head.SetReserved(reserved);
		return ;
	}	
	

/// ȡЭ��ͷ��Ϣ�Ľӿ�
public:	
	inline unsigned char version()
	{
		return m_pPacket->head.version;
	}
	inline unsigned int cmd()
	{
		return m_pPacket->head.cmd;
	}
	inline unsigned int serialNo()
	{
		return m_pPacket->head.serialNo;
	}
	inline unsigned int colorration()
	{
		return m_pPacket->head.colorration;
	}
	inline unsigned char serverResponseFlag()
	{
		return m_pPacket->head.serverResponseFlag;
	}
	inline unsigned short serverResponseInfo()
	{
		return m_pPacket->head.serverResponseInfo;
	}
	inline unsigned int len()
	{
		return m_pPacket->head.len;
	}

/// ȡЭ�鳤�ȡ�λ����صĽӿ�
public:
	inline int bodySize()
	{
		return m_iTail;
	}
	inline int bodySize(unsigned int iPacketLen)
	{
		return (iPacketLen - sizeof(char) - sizeof(QzoneProtocolHead) - sizeof(char));
	}
	inline int bodyMaxLen()
	{
		return m_iBodyMaxLen;
	}
	inline int bodyPos()
	{
		return m_iPos;
	}
	inline int headSize()
	{
		return (sizeof(char) + sizeof(QzoneProtocolHead));
	}
	/*
	@brief: �õ�packet��ָ��, ���Զ�������ⲿ����, �����ʹ��
	*/
    inline char* packet()
    {
        return (char*)m_pPacket;
    }

	inline char* body()
	{
		return (char*)m_pPacket->body;
	}
	

/// ӳ��Э��ͷbuffer��ȡЭ��ͷ��Ϣ�Ľӿ�
public:
	inline int MapPacketHeadBuffer(const char* buf, unsigned int bufLen)
	{
		if (m_pPacket)
			return -1;

		if (bufLen < (sizeof(char) + sizeof(QzoneProtocolHead)))
			return -2;
		
		m_pPacket = (QzoneProtocolPtr) buf;
		m_iBodyMaxLen = bufLen - sizeof(char) - sizeof(QzoneProtocolHead);
		m_iTail = 0;
		m_iPos = 0;	
			
		return 0;
	}
	inline void UnmapPacketHeadBuffer()
	{
		m_pPacket = NULL;
		return ;
	}
	inline unsigned char mappedVersion()
	{
		return m_pPacket->head.version;
	}
	inline unsigned int mappedCmd()
	{
		return ntohl(m_pPacket->head.cmd);
	}
	inline unsigned int mappedSerialNo()
	{
		return ntohl(m_pPacket->head.serialNo);
	}
	inline unsigned int mappedColorration()
	{
		return ntohl(m_pPacket->head.colorration);
	}
	inline unsigned char mappedServerResponseFlag()
	{
		return m_pPacket->head.serverResponseFlag;
	}
	inline unsigned short mappedServerResponseInfo()
	{
		return ntohs(m_pPacket->head.serverResponseInfo);
	}
	inline unsigned int mappedLen()
	{
		return ntohl(m_pPacket->head.len);
	}	
	inline unsigned char mappedReserved()
	{
		return m_pPacket->head.reserved[0];
	}

/// map��Ӧ��packet buffer
public:
	inline int CheckPacketBuffer(const char* buf)
	{
		if (buf[0] != QzoneProtocolSOH)
			return -1;
		
		return 0;
	}
	inline int MapPacketBuffer(const char* buf, unsigned int bufLen)
	{
		if (m_pPacket)
			return -1;

		if (buf == NULL)
			return -2;
		
		m_pPacket = (QzoneProtocolPtr) buf;
		m_iBodyMaxLen = bufLen - sizeof(char) - sizeof(QzoneProtocolHead) - sizeof(char);
		
		return 0;
	}
	inline void UnmapPacketBuffer()
	{		
		m_pPacket = NULL;
		
		return ;
	}

/// ����& �ͷ�& ����packet
public:
	inline int CreatePacket(unsigned int iBodyMaxLen)
	{
		if (m_pPacket)
			return -1;
		
		unsigned int len = sizeof(char) + sizeof(QzoneProtocolHead) + iBodyMaxLen + sizeof(char);
		m_pPacket = (QzoneProtocolPtr) malloc(len * sizeof(char));
		if (m_pPacket)
			memset(m_pPacket, 0, len);
		else
			return -2;
		
		m_iBodyMaxLen = iBodyMaxLen;
		m_iTail = 0;
		m_iPos = 0;

		m_bCreated = true;
        m_bNew = true;
		
		return 0;
	}	

    inline int CreatePacketEx(unsigned int iBodyMaxLen, char* pbuffer)
	{
		if (m_pPacket)
			return -1;
		
		unsigned int len = sizeof(char) + sizeof(QzoneProtocolHead) + iBodyMaxLen + sizeof(char);
		m_pPacket = (QzoneProtocolPtr)pbuffer;
		if (m_pPacket)
			memset(m_pPacket, 0, len);
		else
			return -2;
		
		m_iBodyMaxLen = iBodyMaxLen;
		m_iTail = 0;
		m_iPos = 0;

		m_bCreated = true;
        m_bNew = false;
		
		return 0;
	}	
	inline void FreePacket()
	{
		if (m_bCreated)
		{			
			if(m_pPacket != NULL && m_bNew)
			{
				free(m_pPacket);
				m_pPacket = NULL;				
			}
			m_bCreated = false;
		}

		return ;
	}

    bool GetOutput()
    {
        return m_bOutput;
    }
    
	inline void ResetPacket()
	{
        m_iErrno = 0;
		m_iTail = 0;
		m_iPos = 0;

		m_bInput = false;
		m_bOutput = false;

		return ;
	}


/// ���, �����ؽӿ�
public:
	/*
	@brief:	�����ؽӿ�, ������CreatePacket()
	@param:	packet�ĳ���, �Ƿ���ҪЧ��
	*/
	inline int Input(unsigned int iPacketLen, bool check = true)
	{
		if (m_bInput)
			return 1;

		if (!m_bCreated)
			return 2;

		if (!m_pPacket)
			return -1;

		if ((sizeof(char) + sizeof(QzoneProtocolHead) + m_iBodyMaxLen + sizeof(char)) < iPacketLen)
			return -2;

		if (packet()[0] != QzoneProtocolSOH)
			return -3;

		if (packet()[iPacketLen - 1] != QzoneProtocolEOT)
			return -4;

		m_iPos = 0;
		m_iTail = iPacketLen - (sizeof(char) + sizeof(QzoneProtocolHead) + sizeof(char));

		if (check)
		{
			// checksum		
			unsigned short checksum = m_pPacket->head.CheckSum((char*)m_pPacket, iPacketLen);
			if (checksum != 0x0 && checksum != 0xffff)
				return -5;
		}
		
		m_pPacket->head.Decode();
		m_bInput = true;

		return 0;
	}
	/*
	@brief:	�����ؽӿ�, ���Ƽ�ʹ��, ��Ϊ��Ҫ�ⲿ��֤buffer�Ĵ���
	@param:	���ݰ�buffer, buffer����, �Ƿ���ҪЧ��
	*/
	inline int Input(const char* buf, unsigned int bufLen, bool check = true)
	{
		if (m_bInput)
			return 1;

		if (m_bCreated)
			return 2;
		
		if (buf == NULL)
			return -1;
		
		if (bufLen < (sizeof(char) + sizeof(QzoneProtocolHead) + sizeof(char)))
			return -2;

		if (buf[0] != QzoneProtocolSOH)
			return -3;

		if (buf[bufLen - 1] != QzoneProtocolEOT)
			return -4;
		
		m_pPacket = (QzoneProtocolPtr) buf;		
		m_iPos = 0;
		m_iTail = bufLen - sizeof(char) - sizeof(QzoneProtocolHead) - sizeof(char);
		m_iBodyMaxLen = m_iTail;		

		if (check)
		{
			// checksum		
			unsigned short checksum = m_pPacket->head.CheckSum(buf, bufLen);
			if (checksum != 0x0 && checksum != 0xffff)
				return -5;
		}
		
		m_pPacket->head.Decode();
		m_bInput = true;

		return 0;
	}

	/*
	@brief:	�����ؽӿ�
	@param:	�����bufferָ��, buffer�ĳ���, �Ƿ���ҪЧ��
	*/
	inline int Output(char* &packetBuf, int &packetLen, bool check = true)
	{
		if (m_bOutput)
		{
			packetBuf = (char*) m_pPacket;
			packetLen = sizeof(char) + sizeof(QzoneProtocolHead) + m_iTail + sizeof(char);
			return 1;
		}
		
		// set soh & eot
		*soh() = QzoneProtocolSOH;
		*eot() = QzoneProtocolEOT;

		// set len
		packetLen = sizeof(char) + sizeof(QzoneProtocolHead) + m_iTail + sizeof(char);		
		m_pPacket->head.SetLen(packetLen);		

		// encode head
		m_pPacket->head.Encode();
		packetBuf = (char*) m_pPacket;
		
		// set checksum
		if (check)
		{
			m_pPacket->head.checksum = 0;
			m_pPacket->head.checksum = m_pPacket->head.CheckSum(packetBuf, packetLen);
		}
		
		m_bOutput = true;

		return 0;
	}

/// ����/ȡ�����ݽӿ�
public:
	inline bool IsProtocolError()
	{
		if (m_iErrno == 0)
			return false;
		else
			return true;
	}
	inline void ResetProtocolError()
	{
		m_iErrno = 0;
		return ;
	}
	inline void SetProtocolError()
	{
		m_iErrno = -100;
		return ;
	}
	
	inline int AppendByte(char cByteParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(char)) > m_iBodyMaxLen)
			return (m_iErrno = -1);
		
		m_pPacket->body[m_iPos] = cByteParam;
		m_iPos += sizeof(char);

		//�����βС�ڵ�ǰλ�ã���������
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}
		return 0;
	}	
	inline int FetchByte(char &cByteParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(char)) > m_iTail)
			return (m_iErrno = -1);

		cByteParam = m_pPacket->body[m_iPos];
		m_iPos += sizeof(char);

		return 0;
	}
	
	inline int AppendByte(unsigned char cByteParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned char)) > m_iBodyMaxLen)
			return (m_iErrno = -1);		
		
		m_pPacket->body[m_iPos] = cByteParam;
		m_iPos += sizeof(unsigned char);

		//�����βС�ڵ�ǰλ�ã���������
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}
		return 0;
	}
	inline int FetchByte(unsigned char &cByteParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned char)) > m_iTail)
			return (m_iErrno = -1);

		cByteParam = m_pPacket->body[m_iPos];
		m_iPos += sizeof(unsigned char);

		return 0;
	}

	inline int AppendShort(short usParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(short)) > m_iBodyMaxLen)
			return (m_iErrno = -1);
		
		usParam = htons(usParam);
		memcpy(&m_pPacket->body[m_iPos], &usParam, sizeof(short));
		m_iPos += sizeof(short);		

		//�����βС�ڵ�ǰλ�ã���������
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}		
		return 0;
	}
	inline int FetchShort(short &usParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(short)) > m_iTail)
			return (m_iErrno = -1);

		memcpy(&usParam, &m_pPacket->body[m_iPos], sizeof(short));
		usParam = ntohs(usParam);
		m_iPos += sizeof(short);

		return 0;
	}

	inline int AppendShort(unsigned short usParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned short)) > m_iBodyMaxLen)
			return (m_iErrno = -1);
		
		usParam = htons(usParam);
		memcpy(&m_pPacket->body[m_iPos], &usParam, sizeof(short));
		m_iPos += sizeof(unsigned short);		

		//�����βС�ڵ�ǰλ�ã���������
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}		
		return 0;
	}
	inline int FetchShort(unsigned short &usParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned short)) > m_iTail)
			return (m_iErrno = -1);

		memcpy(&usParam, &m_pPacket->body[m_iPos], sizeof(short));
		usParam = ntohs(usParam);
		m_iPos += sizeof(unsigned short);

		return 0;
	}
	
	inline int AppendInt(int uiParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(int)) > m_iBodyMaxLen)
			return (m_iErrno = -1);
		
		uiParam = htonl(uiParam);
		memcpy(&m_pPacket->body[m_iPos], &uiParam, sizeof(int));
		m_iPos += sizeof(int);

		//�����βС�ڵ�ǰλ�ã���������
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}		
		return 0;
	}
	inline int FetchInt(int &uiParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(int)) > m_iTail)
			return (m_iErrno = -1);

		memcpy(&uiParam, &m_pPacket->body[m_iPos], sizeof(int));
		uiParam = ntohl(uiParam);
		m_iPos += sizeof(int);

		return 0;
	}

	inline int AppendInt(unsigned int uiParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned int)) > m_iBodyMaxLen)
			return (m_iErrno = -1);
		
		uiParam = htonl(uiParam);
		memcpy(&m_pPacket->body[m_iPos], &uiParam, sizeof(int));
		m_iPos += sizeof(unsigned int);

		//�����βС�ڵ�ǰλ�ã���������
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}		
		return 0;
	}
	inline int FetchInt(unsigned int &uiParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned int)) > m_iTail)
			return (m_iErrno = -1);

		memcpy(&uiParam, &m_pPacket->body[m_iPos], sizeof(int));
		uiParam = ntohl(uiParam);
		m_iPos += sizeof(unsigned int);

		return 0;
	}

	//////add by geminiguan
	inline int AppendLongLong(unsigned long long uiParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned long long)) > m_iBodyMaxLen)
			return (m_iErrno = -1);
		
		uiParam = htonq(uiParam);
		memcpy(&m_pPacket->body[m_iPos], &uiParam, sizeof(unsigned long long));
		m_iPos += sizeof(unsigned long long);

		//�����βС�ڵ�ǰλ�ã���������
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}		
		return 0;
	}
	inline int FetchLongLong(unsigned long long &uiParam)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + sizeof(unsigned long long)) > m_iTail)
			return (m_iErrno = -1);

		memcpy(&uiParam, &m_pPacket->body[m_iPos], sizeof(unsigned long long));
		uiParam = htonq(uiParam);
		m_iPos += sizeof(unsigned long long);

		return 0;
	}
	
/////end add by geminiguan

	inline int AppendData(const char* pData, unsigned int iLen)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + iLen) > m_iBodyMaxLen)
			return (m_iErrno = -1);
		
		if (pData == NULL)
			return (m_iErrno = -2);

		memcpy(&m_pPacket->body[m_iPos], pData, iLen);
		m_iPos += iLen;

		//�����βС�ڵ�ǰλ�ã���������
		if (m_iTail < m_iPos)
		{
			m_iTail = m_iPos;
		}		
		return 0;
	}
	inline int FetchData(char *&pData, unsigned int iLen)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + iLen) > m_iTail)
			return (m_iErrno = -1);
		
		//pData = &m_pPacket->body[m_iPos];
		memcpy(pData, &m_pPacket->body[m_iPos], iLen);
		m_iPos += iLen;

		return 0;
	}
	/// ���Ƽ�ʹ��
	inline int FetchDataPos(char *&pData, unsigned int iLen)
	{
		if (m_iErrno != 0)
			return m_iErrno;
		
		if ((m_iPos + iLen) > m_iTail)
			return (m_iErrno = -1);
		
		pData = &m_pPacket->body[m_iPos];
		m_iPos += iLen;
		
		return 0;
	}

private:
	inline char* soh()
	{
		return (char*) m_pPacket;
	}
	inline char* eot()
	{
		char *p = (char*) m_pPacket;
		p += (sizeof(char) + sizeof(QzoneProtocolHead) + m_iTail);
		return p;
	}	

/// ȡserverResponse������Ϣ
public:
	inline void TranslateServerResponseInfo(char * pInfo, unsigned int iInfoLen)
	{
		switch (serverResponseFlag())
		{										
			// ��������, ����ɹ�
			case QzoneServerSucc:
				snprintf(pInfo, iInfoLen, "SUCCESS.");
				return ;

			// ��������, ����ʧ��
			case QzoneServerFailed:
				// ���ڴ���ʧ��(1):  ��ʾ����ʧ�ܵĴ����errcode
				snprintf(pInfo, iInfoLen, "FAILED, ERRCODE=%d.", serverResponseInfo());
				return ;

			// �쳣����, �������ܾ�����
			case QzoneServerExc:
				snprintf(pInfo, iInfoLen, "DATA EXC, ERRINFO=%d.", serverResponseInfo());
				return ;

			// ��������, ������æ, ������
			case QzoneServerBusy:
				// ���ڷ�����æ(3):  ��ʾ����ʱ��
				snprintf(pInfo, iInfoLen, "SERVER BUSY, RETRYTIME=%d.", serverResponseInfo());
				return ;

			// �������ض���
			case QzoneServerRedirected:
				// �������ض���
				snprintf(pInfo, iInfoLen, "SERVER REDIRECTED.");
				return ;

			// ��ִ��
			case QzoneServerAck:
				snprintf(pInfo, iInfoLen, "ACK.");
				return ;

			// client����, ��server��Ӧ
			case QzoneClient:
				snprintf(pInfo, iInfoLen, "CLIENT QUERY.");
				return ;

			default:
				snprintf(pInfo, iInfoLen, "UNKNOWN.");
				return ;
		}

		return ;
	}

	
};

#endif

