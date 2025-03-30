#include <stdarg.h>
#include <string.h>
#include "gtEncode.h"
#include "gtMain.h"

#if defined(__mips64) || defined(UNDER_UNIX)

#define GT_ENCODE_ELEM(TYPE,b,l,args) \
{ \
	TYPE v; \
	if (l < sizeof(TYPE)) \
		return -1; \
	v = va_arg(*args, int); \
	memcpy(b, &v, sizeof(TYPE)); \
	return sizeof(TYPE); \
}

#define GT_DECODE_ELEM(TYPE,b,l,args) \
{ \
	TYPE* v; \
	if (l < sizeof(TYPE)) \
		return -1; \
	v = va_arg(*args, TYPE*); \
	memcpy(v, b, sizeof(TYPE)); \
	return sizeof(TYPE); \
}

#else

#define GT_ENCODE_ELEM(TYPE,b,l,args) {if (l < sizeof(TYPE)) return -1; memcpy(b,&va_arg(*args,TYPE),sizeof(TYPE)); return sizeof(TYPE);}
#define GT_DECODE_ELEM(TYPE,b,l,args) {if (l < sizeof(TYPE)) return -1; memcpy(va_arg(*args,TYPE*),b,sizeof(TYPE)); return sizeof(TYPE);}

#endif /* __mips64 */

static int dbstrlen(GT_DBSTR_TYPE dbstr)
{
	int len = 0;
#ifdef ALIGNED_COPY
	short achar;
	do
	{
		memcpy(&achar, dbstr, sizeof(achar));
		dbstr++;
		len++;
	} while (achar != 0);
	len--;
#else
	while (*dbstr++)
		len++;
#endif
	return len;
}

static short *dbstrcpy(GT_DBSTR_TYPE dest, GT_DBSTR_TYPE src)
{
	GT_DBSTR_TYPE hold = dest;
	#ifdef ALIGNED_COPY
		int len = dbstrlen(src);
		memcpy(dest, src, (len + 1) * 2);
	#else
		while ((*dest++ = *src++) != 0);
	#endif
	return hold;
}

static int gtiDecodeBits(int bitcount, char *inBuffer, int inLength, va_list *args)
{
	char bucket;
	int i;

	if (inLength < 1)
		return -1;
	bucket = *inBuffer;
	for (i = 0 ; i < bitcount ; i++)
	{
		*va_arg(*args,char*) = (bucket & (1 << i)) ? 1 : 0;
	}
	
	return 1;	
}

static int gtiEncodeBits(int bitcount, char *outBuffer, int outLength, va_list *args)
{
	char bucket = 0;
	int i;
	
	if (outLength < 1)
		return -1;
	for (i = 0 ; i < bitcount ; i++)
	{
		bucket |= ((va_arg(*args,int) ? 1 : 0) << i);
		//bucket |= ((va_arg(*args,char) ? 1 : 0) << i);
	}
	*outBuffer = bucket;
	return 1;
}

static int gtiDecodeSingle(char elemType, char *inBuffer, int inLength, va_list *args)
{
	switch (elemType)
	{
	case GT_INT:
		GT_DECODE_ELEM(GT_INT_TYPE,inBuffer, inLength, args);
		break;
	case GT_UINT:
		GT_DECODE_ELEM(GT_UINT_TYPE,inBuffer, inLength, args);
		break;
	case GT_SHORT:
		GT_DECODE_ELEM(GT_SHORT_TYPE,inBuffer, inLength, args);
		break;
	case GT_USHORT:
		GT_DECODE_ELEM(GT_USHORT_TYPE,inBuffer, inLength, args);
		break;
	case GT_CHAR:
		GT_DECODE_ELEM(GT_CHAR_TYPE,inBuffer, inLength, args);
		break;
	case GT_UCHAR:
		GT_DECODE_ELEM(GT_UCHAR_TYPE,inBuffer, inLength, args);
		break;
	case GT_FLOAT: 
		GT_DECODE_ELEM(GT_FLOAT_TYPE,inBuffer, inLength, args);
		break;
	case GT_DOUBLE:
		GT_DECODE_ELEM(GT_DOUBLE_TYPE,inBuffer, inLength, args);
		break;
	case GT_BIT:
		GT_DECODE_ELEM(GT_BIT_TYPE,inBuffer, inLength, args);
		break;
	case GT_CSTR:
		{
			int len;
			GT_CSTR_TYPE s = va_arg(*args, GT_CSTR_TYPE);
			assert(s != NULL);
			len = strlen(inBuffer) + 1;
			if (inLength < len )
				return -1;
			strcpy(s, inBuffer);
			return len;
		}
		break;
	case GT_CSTR_PTR:
		*va_arg(*args, GT_CSTR_PTR_TYPE) = (GT_CSTR_TYPE)inBuffer;
		return strlen(inBuffer) + 1;
		break;
	case GT_DBSTR:
		{	
			int len;
			GT_DBSTR_TYPE s = va_arg(*args, GT_DBSTR_TYPE);
			assert(s != NULL);
			len = dbstrlen((GT_DBSTR_TYPE)inBuffer) + 1;
			if (inLength < len * 2)
				return -1;
			dbstrcpy(s, (GT_DBSTR_TYPE)inBuffer);
			return len * 2;
		}
		break;
	case GT_DBSTR_PTR:
		*va_arg(*args, GT_DBSTR_PTR_TYPE) = (GT_DBSTR_TYPE)inBuffer;
		return (dbstrlen((GT_DBSTR_TYPE)inBuffer) + 1) * 2;
		break;
	case GT_RAW:
		{
			int *len, holdlen;
			GT_RAW_TYPE data = va_arg(*args, GT_RAW_TYPE);
			len = va_arg(*args, int *);
			if (inLength < sizeof(*len))
				return -1;
			holdlen = *len;
			memcpy(len, inBuffer, sizeof(*len));
			if (*len > holdlen) //there isn't enough room in their dest!
				return -1;
			if (inLength < (int)sizeof(*len) + *len)
				return -1;
			memcpy(data, inBuffer + sizeof(*len), *len);
			return *len + sizeof(*len);
		}
	case GT_RAW_PTR:
		{
			int *len;
			*va_arg(*args, GT_RAW_PTR_TYPE) = (GT_RAW_TYPE)(inBuffer + sizeof(*len));
			len = va_arg(*args, int *);
			if (inLength < sizeof(*len))
				return -1;
			memcpy(len, inBuffer, sizeof(*len));
			return *len + sizeof(*len);
		}
		break;
		
	}
	return -1; //bad type!
}

static int gtiEncodeSingle(char elemType, char *outBuffer, int outLength, va_list *args)
{
	switch (elemType)
	{
	case GT_INT:
		GT_ENCODE_ELEM(GT_INT_TYPE,outBuffer, outLength, args);
		break;
	case GT_UINT:
		GT_ENCODE_ELEM(GT_UINT_TYPE,outBuffer, outLength, args);
		break;
	case GT_SHORT:
		GT_ENCODE_ELEM(GT_SHORT_TYPE,outBuffer, outLength, args);
		break;
	case GT_USHORT:
		GT_ENCODE_ELEM(GT_USHORT_TYPE,outBuffer, outLength, args);
		break;
	case GT_CHAR:
		GT_ENCODE_ELEM(GT_CHAR_TYPE,outBuffer, outLength, args);
		break;
	case GT_UCHAR:
		GT_ENCODE_ELEM(GT_UCHAR_TYPE,outBuffer, outLength, args);
		break;
	case GT_FLOAT: //floats are promoted to double in varargs, need to demote
		{
			double temp;
			float f;
			double v = va_arg(*args,double);
			memcpy(&temp,&v,sizeof(double));
			f = (float)temp;
			if (outLength < sizeof(float))
				return -1;
			memcpy(outBuffer, &f, sizeof(float));
			return sizeof(float);			
		}
		break;
	case GT_DOUBLE:
		{
			double v;
			if(outLength < sizeof(double))
				return -1;
			v = va_arg(*args, double);
			memcpy(outBuffer, &v, sizeof(double));
			return sizeof(double);
		}
		break;
	case GT_BIT:
		GT_ENCODE_ELEM(GT_BIT_TYPE,outBuffer, outLength, args);
		break;
	case GT_CSTR:
	case GT_CSTR_PTR:
		{
			int len;
			GT_CSTR_TYPE s = va_arg(*args, GT_CSTR_TYPE);
			assert(s != NULL);
			len = strlen(s) + 1;
			if (outLength < len )
				return -1;
			strcpy(outBuffer, s);
			return len;
		}
		break;
	case GT_DBSTR:
	case GT_DBSTR_PTR:
		{	
			int len;
			GT_DBSTR_TYPE s = va_arg(*args, GT_DBSTR_TYPE);
			assert(s != NULL);
			len = dbstrlen(s) + 1;
			if (outLength < len * 2)
				return -1;
			dbstrcpy((short *)outBuffer, s);
			return len * 2;
		}
		break;
	case GT_RAW:
	case GT_RAW_PTR:
		{
			int len;
			GT_RAW_TYPE data = va_arg(*args, GT_RAW_TYPE);
			len = va_arg(*args, int);
			if (outLength < len + (int)sizeof(len))
				return -1;
			memcpy(outBuffer, &len, sizeof(len));
			memcpy(outBuffer + sizeof(len), data, len);
			return len + sizeof(len);
		}
		
	}
	return -1; //bad type!
}

static int gtInternalEncodeV(int usetype, GTMessageType msgType, const char *fmtString, char *outBuffer, int outLength, va_list *args)
{
	int elemSize;
	int totSize = outLength;
	const char *bitCounter;

	//set the message type
	if (usetype)
	{
		elemSize = sizeof(msgType);
		if (outLength < elemSize)
			return -1;
		memcpy(outBuffer, &msgType, elemSize);
		outBuffer += elemSize;
		outLength -= elemSize;
	}
	while (*fmtString)
	{
		if (*fmtString == GT_BIT) //see how many
		{
			for (bitCounter = fmtString; *bitCounter == GT_BIT && bitCounter - fmtString <= 8; bitCounter++);
			elemSize = gtiEncodeBits(bitCounter - fmtString, outBuffer, outLength, args);
			fmtString = bitCounter - 1;
		} else
			elemSize = gtiEncodeSingle(*fmtString, outBuffer, outLength, args);
		if (elemSize < 0)
			return -1; //out of space
		outBuffer += elemSize;
		outLength -= elemSize;
		fmtString++;
	}
   	return totSize - outLength;
}

int gtEncodeNoTypeV(const char *fmtString, char *outBuffer, int outLength, va_list *args)
{
	return gtInternalEncodeV(0,0,fmtString, outBuffer, outLength, args);
}

int gtEncodeV(GTMessageType msgType, const char *fmtString, char *outBuffer, int outLength, va_list *args)
{
	return gtInternalEncodeV(1,msgType,fmtString, outBuffer, outLength, args);
}

int gtEncode(GTMessageType msgType, const char *fmtString, char *outBuffer, int outLength, ...)
{
	int rcode;
	va_list args;

	//set the values
	va_start(args, outLength);
	rcode = gtEncodeV(msgType, fmtString, outBuffer, outLength, &args);
	va_end(args);

	return rcode;
}

int gtEncodeNoType(const char *fmtString, char *outBuffer, int outLength, ...)
{
	int rcode;
	va_list args;
	
	//set the values
	va_start(args, outLength);
	rcode = gtEncodeNoTypeV(fmtString, outBuffer, outLength, &args);
	va_end(args);

	return rcode;
}

static int gtDecodeInternalV(int usetype, const char *fmtString, char *inBuffer, int inLength, 	va_list *args)
{
	int elemSize;
	int totSize = inLength;
	const char *bitCounter;

	//skip the message type
	if (usetype)
	{
		inBuffer += sizeof(GTMessageType);
		inLength -= sizeof(GTMessageType);
	}

	while (*fmtString)
	{
		if (*fmtString == GT_BIT) //see how many
		{
			for (bitCounter = fmtString; *bitCounter == GT_BIT && bitCounter - fmtString <= 8; bitCounter++);
			elemSize = gtiDecodeBits(bitCounter - fmtString, inBuffer, inLength, args);
			fmtString = bitCounter - 1;
		} else
			elemSize = gtiDecodeSingle(*fmtString, inBuffer, inLength, args);
		if (elemSize < 0)
			return -1; //out of space
		inBuffer += elemSize;
		inLength -= elemSize;
		fmtString++;
	}
	//NOTE: inLength should be 0 here if we "ate" the whole message
	//If it's not 0, then the encoding and decoding strings probably did not match
	//which would generally indicate a bug
	//PANTS - commented out because we could be decoding the rest with a gtDecodeNoType
//	assert(inLength == 0);
	return totSize - inLength;
}

int gtDecodeV(const char *fmtString, char *inBuffer, int inLength, 	va_list *args)
{
	return gtDecodeInternalV(1,fmtString, inBuffer, inLength, args);	
}

int gtDecodeNoTypeV(const char *fmtString, char *inBuffer, int inLength, 	va_list *args)
{
	return gtDecodeInternalV(0,fmtString, inBuffer, inLength, args);
}

int gtDecode(const char *fmtString, char *inBuffer, int inLength, ...)
{
	int rcode;
	va_list args;
	
	//set the values
	va_start(args, inLength);
	rcode = gtDecodeV(fmtString, inBuffer, inLength, &args);
	va_end(args);

	return rcode;
}

int gtDecodeNoType(const char *fmtString, char *inBuffer, int inLength, ...)
{
	int rcode;
	va_list args;
	
	//set the values
	va_start(args, inLength);
	rcode = gtDecodeNoTypeV(fmtString, inBuffer, inLength, &args);
	va_end(args);

	return rcode;
}

GTMessageType gtEncodedMessageType(char *inBuffer)
{ //TODO: check for byte alignment?
	return *(GTMessageType *)inBuffer;
}