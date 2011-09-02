#include "qza_qzone_converter.h"
const int RTN_ERROR = -20;
#define HEADPTR(buf) (((QzoneProtocolPtr)(buf))->head)
#define ERRMSG(fmt, arg...) do{errmsg.resize(128);int __len = snprintf((char*)errmsg.data(), 127, "[%s %s %d]" fmt, __FILE__, __FUNCTION__, __LINE__, ##arg);errmsg.resize(__len);}while(0)
#define TRACE(fmt, arg...) 

#define QZA_CMD_OF_ENTRY_RIGHT		0x05
const int QzoneHeadLen = sizeof(QzoneProtocol) + sizeof(char);
inline unsigned GetQzoneLen(const void *buf){return ntohl(HEADPTR(buf).len);}
int protocol::CheckQzoneProtocol(const void *qzone_packet, int len, string& errmsg)
{
    if(!qzone_packet)
    {
        ERRMSG("argment is null.");
        return RTN_ERROR;
    }
    char* buf = (char*)qzone_packet;
    if(*buf != QzoneProtocolSOH)
    {
        ERRMSG("beginning flag unexpected. begin:0x%x, expected-beginning:0x%x, buf-len:%d", *buf, QzoneProtocolSOH, len);
        return RTN_ERROR;
    }
    if(len < QzoneHeadLen )
    {
        TRACE("buf size %d less than qzone protocol size %d.continue recieve...", len, sizeof(QzoneProtocol));
        return 0;
    }
    int _packet_len = (int)GetQzoneLen(qzone_packet);
    if(_packet_len < QzoneHeadLen)
    {
        ERRMSG("packet-len field invalid. packet-len:%d buf-len:%d", _packet_len, len);
        return RTN_ERROR;
    }
    if(_packet_len > len)
    {
        TRACE("buf size %d less than packet size %d.continue recieve...", len, GetQzoneLen(qzone_packet));
        return 0;
    }
    if(buf[_packet_len - 1] != QzoneProtocolEOT)
    {
        ERRMSG("end flag unexpected. end:0x%x, expected-end:0x%x, buf-len:%d", buf[_packet_len - 1], QzoneProtocolEOT, len);
        return RTN_ERROR;
    }
    return _packet_len;
}
int protocol::CheckQzaProtocol(const void *buf, int len, string &errmsg)
{
    if(!buf)
    {
        ERRMSG("argment is null.");
        return RTN_ERROR;
    }
    if(len < (int)sizeof(QZAHEAD))
    {
        TRACE("buf size %d less than packet size %d.continue recieve...", len, sizeof(QZAHEAD));
        return 0;
    }
    int _packet_len = ((QZAHEAD*)buf)->GetPackLen();
    if(_packet_len > len)
    {
        TRACE("continue recv. len:%d buf:%d", _packet_len, len);
        return 0;
    }
    return _packet_len;
}
int protocol::Qzone2Qza(const void *qzone_packet, int qzone_len, void *outbuf_qza, int buf_size, string &errmsg)
{
    QZAHEAD* _pkg = (QZAHEAD*)outbuf_qza;
    QzoneProtocolPtr _qzonepkg = (QzoneProtocolPtr)qzone_packet;
    _pkg->_version = 8;
    _pkg->_pack_flow = _qzonepkg->head.serialNo;
    _pkg->_rsp_code = _qzonepkg->head.serverResponseInfo;
    _pkg->_detail_info._pack_len_high = _pkg->_detail_info._extend_len = 0;
    _pkg->_req_cmd = QZA_CMD_OF_ENTRY_RIGHT;
    _pkg->_detail_info._req_cmd_item = _qzonepkg->head.cmd;
    int _bodylen = GetQzoneLen(qzone_packet) - QzoneHeadLen;
    if(int(_bodylen + sizeof(QZAHEAD)) > buf_size)
    {
        ERRMSG("not enough buf for qza protocol convert. need:%d",  _bodylen + sizeof(QZAHEAD));
        return RTN_ERROR;
    }
    if(_bodylen)
    {
        memcpy(_pkg->GetBody(), _qzonepkg->body, _bodylen);
    }
    _pkg->SetPackLen(_bodylen + sizeof(QZAHEAD));
    return 0;
}
int protocol::Qza2Qzone(const void *qza_packet, int qza_len, void *outbuf_qzone, int buf_size, string &errmsg)
{

    QZAHEAD* _pkg = (QZAHEAD*)qza_packet;
    int _bodylen = _pkg->GetPackLen() - _pkg->GetHeadLen();
    if(_bodylen + QzoneHeadLen > buf_size)
    {
        ERRMSG("not enough buf for qzone protocol convert. need:%d",  _bodylen + QzoneHeadLen);
        return RTN_ERROR;
    }
    QzoneProtocolPtr _qzonepkg = (QzoneProtocolPtr)outbuf_qzone;
    _qzonepkg->soh = QzoneProtocolSOH;
    unsigned _cmd = ntohs(_pkg->_detail_info._req_cmd_item);
    _qzonepkg->head.cmd = htonl(_cmd);
    _qzonepkg->head.serialNo = _pkg->_pack_flow;
    _qzonepkg->head.serverResponseInfo = 0;
    _qzonepkg->head.serverResponseFlag = 0;

    memcpy(_qzonepkg->body, _pkg->GetBody(), _bodylen);
    _qzonepkg->body[_bodylen] = QzoneProtocolEOT;
    _qzonepkg->head.len = htonl(_bodylen + QzoneHeadLen);
    return 0;
}
void protocol::GetIpInfo(const void *qza_packet, int qza_len, string &out_ipinfo)
{
    QZAHEAD* _pkg = (QZAHEAD*)qza_packet;
    out_ipinfo.resize(32);
    if(_pkg->_detail_info._client_ip)
    {
        out_ipinfo.append("client:");
        out_ipinfo.append(inet_ntoa(*(struct in_addr*)&_pkg->_detail_info._client_ip));
    }
    if(_pkg->_detail_info._req_ip)
    {
        out_ipinfo.append("req:");
        out_ipinfo.append(inet_ntoa(*(struct in_addr*)&_pkg->_detail_info._req_ip));
    }
}

