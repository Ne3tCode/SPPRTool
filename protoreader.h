#ifndef PROTOREADER_H
#define PROTOREADER_H

unsigned int ReadProtoNumber(const unsigned char *pubInput, unsigned int cubAvail, void *pubDest, unsigned int cubSize, bool bFixed = false);
unsigned int ReadProtoLengthDelimited(const unsigned char *pubInput, unsigned int cubAvail, void *pubDest = NULL, unsigned int *cubSize = NULL);
unsigned int ReadProtoField(const unsigned char *pubInput, unsigned int cubAvail, unsigned int *punTag, unsigned int *punWireType, void *pubDest = NULL, unsigned int *cubSize = NULL);
bool GetFieldDataByTag(unsigned int nTag, const unsigned char *pubInput, unsigned int cubAvail, void *pubDest, unsigned int *cubSize, unsigned int *cubTotalRead = NULL);

#endif // PROTOREADER_H
