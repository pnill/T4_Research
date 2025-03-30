/*****************************************************************
gsmcodec.c
GameSpy Voice SDK
Richard "Huckster" Rice

Copyright 2002 GameSpy Industries, Inc.

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com

********************************************************************
Please see the GameSpy Voice SDK documentation for more information
*******************************************************************/
#include "gsmcodec.h"
#include "gsm.h"

// Define GSI_UNUSED, this is in nonport.h but not included because of GSI_MEM_ONLY
#ifdef __MWERKS__
#define GSI_UNUSED(a) (a)
#else
#define GSI_UNUSED(a)
#endif

static CodecState Create(CodecStateType type)
{
	GSI_UNUSED(type); // Unused parameter, this is here to quiet compiler warnings
	return (CodecState)gsm_create();
}

static void Destroy(CodecState state)
{
	gsm_destroy((gsm)state);
}

static int Encode(unsigned char *packet, 
			short *buffer,
			CodecState state)
{
	int gsm_lpt = 0;
	gsm_option((gsm)state, GSM_OPT_LTP_CUT, &gsm_lpt);
	return gsm_encode((gsm)state, buffer, packet);
}

static int Decode(short *out, 
		   unsigned char *in, 
		   CodecState state)
{
	int dec_len;
	dec_len = gsm_decode((gsm)state, in, out); 
	//printf("dec_len = %d\n", dec_len);
	return dec_len;
}

static void Cleanup(void)
{

}

VoiceCodec *GSMCodecInit(unsigned int flags)
{
	static VoiceCodec desc;

	desc.id = "A13";
	desc.create_state = Create;
	desc.destroy_state = Destroy;
	desc.encode = Encode;
	desc.decode = Decode;
	desc.cleanup = Cleanup;

	desc.encoded_frame_size = 33;
	desc.samples_per_frame = 160;

	desc.flags = flags;

	VoiceCodecRegister(&desc);

	return &desc;
}
