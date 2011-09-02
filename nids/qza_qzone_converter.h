/******************************************************************************
 * Copyright:      Shenzhen Tecent Co.Ltd.
 * 
 * qza_qzone_converter.h
 * 
 * Description:
 * 
 * --------------------
 * 2011-1-10, blaketang create
 * --------------------
 ******************************************************************************/
#ifndef __QZA_QZONE_CONVERTER_H__011011__
#define __QZA_QZONE_CONVERTER_H__011011__
#include <string>
#include "qzone_protocol.h"
#include "qza_protocol.h"
using namespace std;
namespace protocol
{
	int CheckQzoneProtocol(const void* buf, int len, string& errmsg);
	int CheckQzaProtocol(const void* buf, int len, string& errmsg);
	/// <summary>
	/// convert qzone protocol to qza protocol.  (from qzone_packet buffer, to outbuf_qza)
	/// </summary>
	/// <param name="qzone_packet"></param>
	/// <param name="qzone_len"></param>
	/// <param name="outbuf_qza"></param>
	/// <param name="buf_size"></param>
	/// <returns>outbuf_qza length, if less than 0, error happend</returns>
	int Qzone2Qza(const void* qzone_packet, int qzone_len, void* outbuf_qza, int buf_size, string& errmsg);
	int Qza2Qzone(const void* qza_packet, int qza_len, void* outbuf_qzone, int buf_size, string& errmsg);
	void GetIpInfo(const void* qza_packet, int qza_len, string& out_ipinfo);
}

#endif /* __QZA_QZONE_CONVERTER_H__011011__ */ 
