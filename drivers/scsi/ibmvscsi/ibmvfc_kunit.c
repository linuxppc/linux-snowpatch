// SPDX-License-Identifier: GPL-2.0-or-later
#include <kunit/test.h>
#include <kunit/visibility.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_transport_fc.h>
#include <scsi/fc/fc_els.h>
#include <linux/list.h>
#include "ibmvfc.h"

MODULE_IMPORT_NS("EXPORTED_FOR_KUNIT_TESTING");

/**
 * ibmvfc_handle_fpin_event_test - unit test for IBMVFC_AE_FPIN parts of
 * ibmvfc_handle_async
 * @test: pointer to kunit structure
 *
 * Tests
 * - error returns from ibmvfc_handle_async
 * - statistics updates
 *
 * Return: void
 */
static void ibmvfc_handle_fpin_event_test(struct kunit *test)
{
	u64 *stats[IBMVFC_AE_FPIN_CONGESTION_CLEARED + 1] = { NULL };
	enum ibmvfc_ae_fpin_status fs;
	struct ibmvfc_async_subq crq;
	struct ibmvfc_target *tgt;
	struct ibmvfc_host *vhost;
	struct list_head *queue;
	struct list_head *headp;
	LIST_HEAD(evt_doneq);
	u64 pre, post;


	headp = ibmvfc_get_headp();
	KUNIT_ASSERT_NOT_NULL(test, headp);
	queue = headp->next;
	KUNIT_ASSERT_PTR_NE(test, queue, headp);
	vhost = container_of(queue, struct ibmvfc_host, queue);

	KUNIT_EXPECT_GE(test, vhost->num_targets, 1);
	tgt = list_first_entry(&vhost->targets, struct ibmvfc_target, queue);
	KUNIT_EXPECT_NOT_NULL(test, tgt->rport);

	stats[IBMVFC_AE_FPIN_LINK_CONGESTED] = &tgt->rport->fpin_stats.cn;
	stats[IBMVFC_AE_FPIN_PORT_CONGESTED] = &tgt->rport->fpin_stats.cn;
	stats[IBMVFC_AE_FPIN_PORT_CLEARED] = &tgt->rport->fpin_stats.cn_clear;
	stats[IBMVFC_AE_FPIN_CONGESTION_CLEARED] = &tgt->rport->fpin_stats.cn_clear;
	stats[IBMVFC_AE_FPIN_PORT_DEGRADED] = &tgt->rport->fpin_stats.li;

	for (fs = IBMVFC_AE_FPIN_LINK_CONGESTED; fs <= IBMVFC_AE_FPIN_CONGESTION_CLEARED; fs++) {
		crq.valid = 0x80;
		crq.link_state = IBMVFC_AE_LS_LINK_UP;
		crq.fpin_status = fs;
		crq.event = cpu_to_be16(IBMVFC_AE_FPIN);
		crq.wwpn = cpu_to_be64(tgt->wwpn);
		crq.id.node_name = cpu_to_be64(tgt->ids.node_name);
		pre = *stats[fs];
		ibmvfc_handle_asyncq((struct ibmvfc_crq *)&crq, vhost, &evt_doneq);
		post = *stats[fs];
		KUNIT_EXPECT_EQ(test, post, pre+1);
	}

	/* bad path */
	crq.valid = 0x80;
	crq.link_state = IBMVFC_AE_LS_LINK_UP;
	crq.fpin_status = 0; /* bad value */
	crq.event = cpu_to_be16(IBMVFC_AE_FPIN);
	crq.wwpn = cpu_to_be64(tgt->wwpn);
	crq.id.node_name = cpu_to_be64(tgt->ids.node_name);
	ibmvfc_handle_asyncq((struct ibmvfc_crq *)&crq, vhost, &evt_doneq);
}

/**
 * ibmvfc_async_subq_test - unit test for allocating async subqueue
 * @test: pointer to kunit structure
 *
 * Return: void
 */
static void ibmvfc_async_subq_test(struct kunit *test)
{
	struct ibmvfc_host *vhost;
	struct list_head *queue;
	struct list_head *headp;

	headp = ibmvfc_get_headp();
	queue = headp->next;
	vhost = container_of(queue, struct ibmvfc_host, queue);

	KUNIT_EXPECT_NOT_NULL(test, vhost->scsi_scrqs.async_scrq);
}

/**
 * ibmvfc_noop_test - unit test for VFC_NOOP command
 * @test: pointer to kunit structure
 *
 * Return: void
 */
static void ibmvfc_noop_test(struct kunit *test)
{
	struct ibmvfc_host *vhost;
	struct list_head *queue;
	struct ibmvfc_crq crq;
	struct list_head *headp;
	LIST_HEAD(evtq);

	headp = ibmvfc_get_headp();
	queue = headp->next;
	vhost = container_of(queue, struct ibmvfc_host, queue);

	KUNIT_EXPECT_TRUE(test, ibmvfc_check_caps(vhost, IBMVFC_SUPPORT_NOOP_CMD));

	crq.valid = 0x80;
	crq.format = IBMVFC_VFC_NOOP;
	crq.ioba = cpu_to_be64(NULL);
	ibmvfc_handle_crq(&crq, vhost, &evtq);
}

#define IBMVFC_TEST_FPIN_EXT(fs, ev, stat) {			\
	crq.valid = 0x80;					\
	crq.flags = IBMVFC_ASYNC_IS_FPIN_EXT;			\
	crq.link_state = IBMVFC_AE_LS_LINK_UP;			\
	crq.fpin_status = (fs);					\
	crq.event = cpu_to_be16(IBMVFC_AE_FPIN);		\
	crq.wwpn = cpu_to_be64(tgt->wwpn);			\
	crq.fpin_data.flags = IBMVFC_FPIN_EVENT_TYPE_VALID;	\
	crq.fpin_data.event_type = cpu_to_be16((ev));		\
	pre = READ_ONCE(tgt->rport->fpin_stats.stat);		\
	ibmvfc_handle_asyncq((struct ibmvfc_crq *)&crq, vhost,	\
			     &evt_doneq);			\
	post = READ_ONCE(tgt->rport->fpin_stats.stat);		\
}

/**
 * ibmvfc_extended_fpin_test - unit test for extended FPIN events
 * @test: pointer to kunit structure
 *
 * Tests
 *
 * Return: void
 */
static void ibmvfc_extended_fpin_test(struct kunit *test)
{
	enum ibmvfc_ae_fpin_status fs;
	struct ibmvfc_async_subq_fpin crq;
	struct fc_fpin_stats stats;
	struct ibmvfc_target *tgt;
	struct ibmvfc_host *vhost;
	struct list_head *headp;
	LIST_HEAD(evt_doneq);
	u64 pre, post;

	headp = ibmvfc_get_headp();
	vhost = list_first_entry(headp, struct ibmvfc_host, queue);
	KUNIT_ASSERT_NOT_NULL_MSG(test, vhost, "No vhost");

	KUNIT_ASSERT_GE(test, vhost->num_targets, 1);
	tgt = list_first_entry(&vhost->targets, struct ibmvfc_target, queue);
	KUNIT_ASSERT_NOT_NULL(test, tgt->rport);

	stats = tgt->rport->fpin_stats;

	for (fs = IBMVFC_AE_FPIN_LINK_CONGESTED; fs <= IBMVFC_AE_FPIN_CONGESTION_CLEARED; fs++) {
		switch (fs) {
		case IBMVFC_AE_FPIN_PORT_CLEARED:
		case IBMVFC_AE_FPIN_CONGESTION_CLEARED:
			crq.valid = 0x80;
			crq.flags = IBMVFC_ASYNC_IS_FPIN_EXT;
			crq.link_state = IBMVFC_AE_LS_LINK_UP;
			crq.fpin_status = fs;
			crq.event = cpu_to_be16(IBMVFC_AE_FPIN);
			crq.wwpn = cpu_to_be64(tgt->wwpn);
			crq.fpin_data.flags = IBMVFC_FPIN_EVENT_TYPE_VALID;
			crq.fpin_data.event_type = cpu_to_be16(0);
			pre = READ_ONCE(tgt->rport->fpin_stats.cn_clear);
			ibmvfc_handle_asyncq((struct ibmvfc_crq *)&crq, vhost,
					     &evt_doneq);
			post = READ_ONCE(tgt->rport->fpin_stats.cn_clear);
			break;
		case IBMVFC_AE_FPIN_LINK_CONGESTED:
		case IBMVFC_AE_FPIN_PORT_CONGESTED:
			IBMVFC_TEST_FPIN_EXT(fs, FPIN_CONGN_CLEAR, cn_clear);
			IBMVFC_TEST_FPIN_EXT(fs, FPIN_CONGN_LOST_CREDIT, cn_lost_credit);
			IBMVFC_TEST_FPIN_EXT(fs, FPIN_CONGN_CREDIT_STALL, cn_credit_stall);
			IBMVFC_TEST_FPIN_EXT(fs, FPIN_CONGN_OVERSUBSCRIPTION, cn_oversubscription);
			IBMVFC_TEST_FPIN_EXT(fs, FPIN_CONGN_DEVICE_SPEC, cn_device_specific);
			break;
		case IBMVFC_AE_FPIN_PORT_DEGRADED:
			IBMVFC_TEST_FPIN_EXT(fs, FPIN_LI_UNKNOWN, li_failure_unknown);
			IBMVFC_TEST_FPIN_EXT(fs, FPIN_LI_LINK_FAILURE, li_link_failure_count);
			IBMVFC_TEST_FPIN_EXT(fs, FPIN_LI_LOSS_OF_SYNC, li_loss_of_sync_count);
			IBMVFC_TEST_FPIN_EXT(fs, FPIN_LI_LOSS_OF_SIG, li_loss_of_signals_count);
			IBMVFC_TEST_FPIN_EXT(fs, FPIN_LI_PRIM_SEQ_ERR, li_prim_seq_err_count);
			IBMVFC_TEST_FPIN_EXT(fs, FPIN_LI_INVALID_TX_WD, li_invalid_tx_word_count);
			IBMVFC_TEST_FPIN_EXT(fs, FPIN_LI_INVALID_CRC, li_invalid_crc_count);
			IBMVFC_TEST_FPIN_EXT(fs, FPIN_LI_DEVICE_SPEC, li_device_specific);
			break;
		}
	}
}

static struct kunit_case ibmvfc_fpin_test_cases[] = {
	KUNIT_CASE(ibmvfc_handle_fpin_event_test),
	KUNIT_CASE(ibmvfc_noop_test),
	KUNIT_CASE(ibmvfc_async_subq_test),
	KUNIT_CASE(ibmvfc_extended_fpin_test),
	{},
};

static struct kunit_suite ibmvfc_fpin_test_suite = {
	.name = "ibmvfc-fpin-test",
	.test_cases = ibmvfc_fpin_test_cases,
};
kunit_test_init_section_suite(ibmvfc_fpin_test_suite);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dave Marquardt <davemarq@linux.ibm.com>");
MODULE_DESCRIPTION("Test module for IBM Virtual Fibre Channel Driver");
