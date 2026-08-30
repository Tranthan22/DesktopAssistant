//	*** BAN SAO DONG BO tu Station/src/Link/StationLinkCore.cpp - SUA O CA HAI NOI ***
//-----------------------------------
//	StationLinkCore - implement framing/CRC/parser (host-testable)
//-----------------------------------

#include "StationLinkCore.h"

enum { ST_SOF = 0, ST_SEQ, ST_CMD, ST_LEN_L, ST_LEN_H, ST_PAYLOAD, ST_CRC_H, ST_CRC_L };

uint16_t Link_Crc16(const uint8_t* p_Data, uint16_t p_Len)
{
    uint16_t f_Crc = 0xFFFF;

    for (uint16_t i = 0; i < p_Len; i++)
    {
        f_Crc ^= (uint16_t)p_Data[i] << 8;
        for (uint8_t b = 0; b < 8; b++)
        {
            f_Crc = (f_Crc & 0x8000) ? (uint16_t)((f_Crc << 1) ^ 0x1021) : (uint16_t)(f_Crc << 1);
        }
    }
    return f_Crc;
}

uint16_t Link_BuildFrame(uint8_t* p_Out, uint8_t p_Seq, uint8_t p_Cmd, const uint8_t* p_Payload, uint16_t p_Len)
{
    uint16_t f_Crc = 0;

    if (p_Len > LINK_MAX_PAYLOAD)
    {
        return 0;
    }

    p_Out[0] = LINK_SOF;
    p_Out[1] = p_Seq;
    p_Out[2] = p_Cmd;
    p_Out[3] = (uint8_t)(p_Len & 0xFF);
    p_Out[4] = (uint8_t)(p_Len >> 8);
    for (uint16_t i = 0; i < p_Len; i++)
    {
        p_Out[5 + i] = p_Payload[i];
    }

    f_Crc = Link_Crc16(&p_Out[1], (uint16_t)(4 + p_Len));   // SEQ..payload
    p_Out[5 + p_Len] = (uint8_t)(f_Crc >> 8);
    p_Out[6 + p_Len] = (uint8_t)(f_Crc & 0xFF);

    return (uint16_t)(p_Len + LINK_FRAME_OVERHEAD);
}

void Link_ParserReset(LinkParser* p_Parser)
{
    p_Parser->State = ST_SOF;
    p_Parser->Len = 0;
    p_Parser->Index = 0;
}

uint8_t Link_ParserFeed(LinkParser* p_Parser, uint8_t p_Byte)
{
    switch (p_Parser->State)
    {
    case ST_SOF:
        if (p_Byte == LINK_SOF) { p_Parser->State = ST_SEQ; }
        break;
    case ST_SEQ:
        p_Parser->Seq = p_Byte;
        p_Parser->State = ST_CMD;
        break;
    case ST_CMD:
        p_Parser->Cmd = p_Byte;
        p_Parser->State = ST_LEN_L;
        break;
    case ST_LEN_L:
        p_Parser->Len = p_Byte;
        p_Parser->State = ST_LEN_H;
        break;
    case ST_LEN_H:
        p_Parser->Len |= (uint16_t)p_Byte << 8;
        if (p_Parser->Len > LINK_MAX_PAYLOAD)
        {
            Link_ParserReset(p_Parser);                     // do dai vo ly -> tim SOF lai
            break;
        }
        p_Parser->Index = 0;
        p_Parser->State = (p_Parser->Len > 0) ? ST_PAYLOAD : ST_CRC_H;
        break;
    case ST_PAYLOAD:
        p_Parser->Payload[p_Parser->Index++] = p_Byte;
        if (p_Parser->Index >= p_Parser->Len) { p_Parser->State = ST_CRC_H; }
        break;
    case ST_CRC_H:
        p_Parser->Crc = (uint16_t)p_Byte << 8;
        p_Parser->State = ST_CRC_L;
        break;
    case ST_CRC_L:
    {
        uint8_t f_Header[4];
        uint16_t f_Crc;

        p_Parser->Crc |= p_Byte;
        p_Parser->State = ST_SOF;

        f_Header[0] = p_Parser->Seq;
        f_Header[1] = p_Parser->Cmd;
        f_Header[2] = (uint8_t)(p_Parser->Len & 0xFF);
        f_Header[3] = (uint8_t)(p_Parser->Len >> 8);
        f_Crc = Link_Crc16(f_Header, 4);
        // CRC noi tiep: header truoc, payload sau (tuong duong tinh 1 mach tren SEQ..payload)
        for (uint16_t i = 0; i < p_Parser->Len; i++)
        {
            f_Crc ^= (uint16_t)p_Parser->Payload[i] << 8;
            for (uint8_t b = 0; b < 8; b++)
            {
                f_Crc = (f_Crc & 0x8000) ? (uint16_t)((f_Crc << 1) ^ 0x1021) : (uint16_t)(f_Crc << 1);
            }
        }
        if (f_Crc == p_Parser->Crc)
        {
            return 1;
        }
        break;
    }
    default:
        Link_ParserReset(p_Parser);
        break;
    }
    return 0;
}
