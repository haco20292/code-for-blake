#ifndef _QZA_PROTOCOL_H_20101019__
#define _QZA_PROTOCOL_H_20101019__
#include <string>
#include <string.h>
#include <sys/types.h>
#include <endian.h>
#include <byteswap.h>
#include <netinet/in.h>
#if BYTE_ORDER == LITTLE_ENDIAN
#undef htonq
#define htonq(x) bswap_64(x)
#elif BYTE_ORDER == BIG_ENDIAN
#undef htonq
#define htonq(x)  (x)
#endif
using namespace std;

#pragma pack(1)

#undef GW_PROTOCOL_STX
#define GW_PROTOCOL_STX		0x8a

typedef struct DETAILINFOtag			/* �ṹ���С: 95�ֽ�*/
{
	char			_version;			//�汾��
	
	unsigned char	_type_platform;		//ƽ̨����	ƽ̨������ڴ��ֶ�ͳ��
	unsigned int	_type_app;			//Ӧ������	ƽ̨������ڴ��ֶ�ͳ��
	unsigned int	_type_source;		//������Դ����, �������������. ���ֶ���ҵ���Լ�����, ƽ̨����������ڴ��ֶ�ͳ��

	char			_reserve_buf[23];	//�����ֶ�
	
	unsigned		_req_ip;			//ҵ�������IP
	unsigned 		_ptlogin_id;		//ҵ����֤��½̬��id
	unsigned char	_auth_type;			//ҵ���½̬keyУ�������
	
	char			_reserve_buf2[9];	//�����ֶ�2
	
	unsigned short	_ptlogin_state;		//��½״̬ 0xffff-δ��֤, 0-�ɹ� 1-ʧ��
	
	short			_req_cmd_item;		//ҵ���������������
	
	unsigned		_client_ip;			//�����ߵĿͻ���IP
	unsigned		_host_uin;			//�������ߵ�UIN
	unsigned		_client_uin;		//�����ߵ�UIN
	
	char			_extend[25];		//��չ����	ͨ��skeyУ���½̬�ĵط���ʹ�������ǰ11���ֽڴ洢skey�����Ϣ
	unsigned short	_pack_len_high;		//������λ ����������==_pack_len + 65536 * _detail_info._pack_len_high;
	unsigned char	_extend_len;		//�䳤������Ϣ����
	
	DETAILINFOtag()
	{
		Clear();
	}
	
	void Clear()
	{
		_version = 0;
		_type_platform = 0;
		_type_app = 0;
		_type_source = 0;
		
		memset(_reserve_buf,0,sizeof(_reserve_buf));
		
		_req_ip = 0;
		_ptlogin_id = 0;
		_auth_type = 0;
		
		memset(_reserve_buf2,0,sizeof(_reserve_buf2));

		_ptlogin_state = 0xFFFF;
		
		_req_cmd_item = 0;
		
		_client_ip = 0;
		_client_uin = 0;
		_host_uin = 0;
		
		memset(_extend,0,sizeof(_extend));
		
		_pack_len_high = 0;
		_extend_len = 0;
	}
	
}DETAILINFO;

typedef struct QZAHEADtag			/* �ṹ���С: 106�ֽ�*/
{
	char			_version;				//�汾��
	unsigned		_pack_flow;				//��ˮ��
	unsigned short	_pack_len;				//���������ȵĵ�λ ����������==_pack_len + 65536 * _detail_info._pack_len_high;
	short			_req_cmd;				//�����������
	short			_rsp_code;				//������
	DETAILINFO		_detail_info;				//������ϸ
	
	QZAHEADtag()
	{
		Clear();
	}
	
	void Clear()
	{
		_version = 0x08;
		_pack_flow = 0;
		_pack_len = 0;
		_req_cmd = 0 ;
		_rsp_code = 0;
		_detail_info.Clear();
	}

	void SetVersion(char v) {_version = v;};
	char GetVersion() {return _version;};

/*
	type_platform;		//ƽ̨����	ƽ̨������ڴ��ֶ�ͳ��
	type_app;			//Ӧ������	ƽ̨������ڴ��ֶ�ͳ��
	type_source;		//������Դ����, �������������. ���ֶ���ҵ���Լ�����, ƽ̨����������ڴ��ֶ�ͳ��
*/
	void SetReqSource(unsigned char type_platform, unsigned int type_app, unsigned int type_source)
	{
		_detail_info._type_platform = type_platform;
		_detail_info._type_app = htonl(type_app);
		_detail_info._type_source = htonl(type_source);
	}
	
	void SetFlow(unsigned flow)	{ _pack_flow = htonl(flow);	};
	void GetFlow(unsigned& flow) { flow = ntohl(_pack_flow); };	
	unsigned GetFlow() { return ntohl(_pack_flow); };
	
	void SetPackLen(unsigned int pack_len)
	{
		_pack_len = htons(pack_len%65536);
		_detail_info._pack_len_high = htons(pack_len/65536);
	};
	void GetPackLen(unsigned int& pack_len) 
	{ 
		pack_len = GetPackLen();
	};
	unsigned int GetPackLen()
	{ 
		return ntohs(_pack_len) + 65536 * ntohs(_detail_info._pack_len_high);
	};
	
	char* GetExtend() { return (char*)&_detail_info._extend[0]; };
	unsigned ExtendSize() { return sizeof(_detail_info._extend); };
	
	void SetExtendLen(unsigned char len) {_detail_info._extend_len = len;}
	unsigned char GetExtendLen() { return _detail_info._extend_len; };
	
	short GetHeadLen() { return (sizeof(QZAHEADtag) + _detail_info._extend_len); };	
	char* GetBody() { return ((char*)this + GetHeadLen()); };
	
	void SetCmd(short cmd,short cmd_item) 
	{
		_req_cmd = htons(cmd);
		_detail_info._req_cmd_item = htons(cmd_item);
	};
	void GetCmd(short& cmd,short& cmd_item)
	{
		cmd = ntohs(_req_cmd);
		cmd_item = ntohs(_detail_info._req_cmd_item);
	}
	
	void SetRspCode(short rsp_code) { _rsp_code = htons(rsp_code); };
	void GetRspCode(short rsp_code) { rsp_code = ntohs(_rsp_code); };
	short GetRspCode() { return ntohs(_rsp_code); };
	
	void SetClient(unsigned client_ip,unsigned client_uin)
	{
		_detail_info._client_ip = htonl(client_ip); 
		_detail_info._client_uin = htonl(client_uin); 
	};
	void GetClient(unsigned& client_ip,unsigned& client_uin)
	{
		client_ip = ntohl(_detail_info._client_ip);
		client_uin = ntohl(_detail_info._client_uin);
	};
	unsigned GetUin() { return ntohl(_detail_info._client_uin); };
	unsigned GetClientUin() { return ntohl(_detail_info._client_uin); };
	
	void SetHostUin(unsigned host_uin) {_detail_info._host_uin = htonl(host_uin);	}
	unsigned GetHostUin() { return ntohl(_detail_info._host_uin); };	
	
	void SetReqIp(unsigned req_ip){ _detail_info._req_ip = htonl(req_ip);}
	unsigned GetReqIp() { return ntohl(_detail_info._req_ip); };

	void SetPtloginId(unsigned ptlogin_id) {_detail_info._ptlogin_id = htonl(ptlogin_id);}
	unsigned GetPtloginId(){return ntohl(_detail_info._ptlogin_id);}

	void SetAuthType(unsigned char auth_type){_detail_info._auth_type = auth_type;};
	unsigned char GetAuthType(){return _detail_info._auth_type;};
	

	/**
	 * ����_detail_info._extend�е�key, key����ܳ���ExtendSize()
	 */
	int SetInternalKey(const string& key)
	{
		unsigned char _len = key.length();
		if(_len + 1 > (unsigned char)ExtendSize())
		{
			return -1;
		}
		_detail_info._extend[0] = _len;
		memcpy(&_detail_info._extend[1], key.data(), _len);
		return 0;
	};
	/**
	 * ȡ_detail_info._extend�е�key
	 */
	string GetInternalKey()
	{
		char _key[ExtendSize()];
		unsigned char _len = _detail_info._extend[0];
		if(_len + 1 > (unsigned char)ExtendSize())
			return "";
		memcpy(_key, &_detail_info._extend[1], _len);
		_key[_len] = 0;
		return _key;
	};

	/**
	 * ���ñ䳤��ͷ�е�key, key����ܳ���254
	 */
	int SetExternalKey(const string& key)
	{
		unsigned int _len = key.length();
		if(_len == 0)
		{
			return 0;
		}
		else if(_len > 254)
		{
			return -1;
		}
		SetExtendLen(0);
		char* _pbuf = GetBody();
		*_pbuf = (unsigned char)_len;
		_pbuf++;
		memcpy(_pbuf, key.data(), _len);
		SetExtendLen(_len + 1);
		return 0;
	};
	/**
	 * ȡ�䳤��ͷ�е�key
	 */
	string GetExternalKey()
	{
		char _key[256];
		char* _pextern = (char*)this + sizeof(QZAHEADtag);
		unsigned char _len = *_pextern;
		_pextern ++;
		memcpy(_key, _pextern, _len);
		_key[_len] = 0;
		return _key;
	};

	/**
	 * ���¸��ֶ�λ�ô��ڸ������(����key�����õ�ͬһλ��, �����ǲ�����ͬһ����ͷ��ͬʱ����)
	 * �밴�������ѡ�����(���ܵ����SetInternalKey�Ĳ�ͬ�ӿڻ���SetExternalKey�Ĳ�ͬ�ӿ�)
	 */
	// skey һ��ΪWEB��������, ��cgi
	int SetSKey(const string& skey){return SetInternalKey(skey);};
	string GetSKey(){return GetInternalKey();};

	// svr key һ��Ϊ�ò����κ�����key�ĳ������
	int SetSvrKey(const string& skey){return SetInternalKey(skey);};
	string GetSvrKey(){return GetInternalKey();};

	// 3g key ���ֻ�����mqzoneʹ�õ�key
	int Set3GKey(const string& a8){	return SetExternalKey(a8);};
	string Get3GKey(){return GetExternalKey();};

	// ����½̬��key
	int SetLSKey(const string& lskey){return SetExternalKey(lskey);};
	string GetLSKey(){return GetExternalKey();};
	
	
}QZAHEAD;

#pragma pack()

#endif

