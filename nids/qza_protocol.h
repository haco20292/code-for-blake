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

typedef struct DETAILINFOtag			/* 结构体大小: 95字节*/
{
	char			_version;			//版本号
	
	unsigned char	_type_platform;		//平台类型	平台会针对于此字段统计
	unsigned int	_type_app;			//应用类型	平台会针对于此字段统计
	unsigned int	_type_source;		//具体来源类型, 例如操作触发地. 此字段由业务自己控制, 平台不单独针对于此字段统计

	char			_reserve_buf[23];	//保留字段
	
	unsigned		_req_ip;			//业务请求的IP
	unsigned 		_ptlogin_id;		//业务验证登陆态的id
	unsigned char	_auth_type;			//业务登陆态key校验的类型
	
	char			_reserve_buf2[9];	//保留字段2
	
	unsigned short	_ptlogin_state;		//登陆状态 0xffff-未验证, 0-成功 1-失败
	
	short			_req_cmd_item;		//业务请求的子命令字
	
	unsigned		_client_ip;			//访问者的客户端IP
	unsigned		_host_uin;			//被访问者的UIN
	unsigned		_client_uin;		//访问者的UIN
	
	char			_extend[25];		//扩展数据	通过skey校验登陆态的地方会使用这里的前11个字节存储skey相关信息
	unsigned short	_pack_len_high;		//包长高位 整个包长度==_pack_len + 65536 * _detail_info._pack_len_high;
	unsigned char	_extend_len;		//变长附加信息长度
	
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

typedef struct QZAHEADtag			/* 结构体大小: 106字节*/
{
	char			_version;				//版本号
	unsigned		_pack_flow;				//流水号
	unsigned short	_pack_len;				//整个包长度的低位 整个包长度==_pack_len + 65536 * _detail_info._pack_len_high;
	short			_req_cmd;				//请求的命令字
	short			_rsp_code;				//返回码
	DETAILINFO		_detail_info;				//请求详细
	
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
	type_platform;		//平台类型	平台会针对于此字段统计
	type_app;			//应用类型	平台会针对于此字段统计
	type_source;		//具体来源类型, 例如操作触发地. 此字段由业务自己控制, 平台不单独针对于此字段统计
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
	 * 设置_detail_info._extend中的key, key最长不能超过ExtendSize()
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
	 * 取_detail_info._extend中的key
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
	 * 设置变长包头中的key, key最长不能超过254
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
	 * 取变长包头中的key
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
	 * 以下各字段位置存在复用情况(如多个key都是用的同一位置, 但它们不会在同一个包头里同时出现)
	 * 请按具体情况选择调用(不能调多次SetInternalKey的不同接口或多次SetExternalKey的不同接口)
	 */
	// skey 一般为WEB层程序调用, 如cgi
	int SetSKey(const string& skey){return SetInternalKey(skey);};
	string GetSKey(){return GetInternalKey();};

	// svr key 一般为拿不到任何其它key的程序调用
	int SetSvrKey(const string& skey){return SetInternalKey(skey);};
	string GetSvrKey(){return GetInternalKey();};

	// 3g key 给手机无线mqzone使用的key
	int Set3GKey(const string& a8){	return SetExternalKey(a8);};
	string Get3GKey(){return GetExternalKey();};

	// 弱登陆态的key
	int SetLSKey(const string& lskey){return SetExternalKey(lskey);};
	string GetLSKey(){return GetExternalKey();};
	
	
}QZAHEAD;

#pragma pack()

#endif

