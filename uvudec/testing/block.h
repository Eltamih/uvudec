/*
UVNet Universal Decompiler (uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef BLOCK_H
#define BLOCK_H

#define BTEST UVCPPUNIT_TEST
//#define BTEST CPPUNIT_TEST

#include "testing/framework/common_fixture.h"
#include "uvd/assembly/address.h"
#include "uvd/core/block.h"

//class BlockFixture : public CPPUNIT_NS::TestFixture {
class BlockFixture : public UVDTestingCommonFixture {
	CPPUNIT_TEST_SUITE(BlockFixture);
	BTEST(addRemoveTest);
	BTEST(findTest);
	BTEST(notifyTest);
	CPPUNIT_TEST_SUITE_END();

public:
	virtual void setUp(void);
	virtual void tearDown(void);
	
	void notifyTest(void);
	void findTest(void);
	void addRemoveTest(void);

private:
	UVDAddressSpace m_space;
	UVDBasicBlock m_block;
};

#endif

