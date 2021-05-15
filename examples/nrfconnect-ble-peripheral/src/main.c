/* main.c - Application main entry point */

/*
 * Copyright (c) 2021
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/printk.h>
#include <sys/util.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <sdc_hci_vs.h>

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL,
		0xBB, 0x4A, 0xFF, 0x4F, 0xAD, 0x03, 0x41, 0x5D,
		0xA9, 0x6C, 0x9D, 0x6C, 0xDD, 0xDA, 0x83, 0x04),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static bool on_vs_evt(struct net_buf_simple *buf)
{
	uint8_t code;
	sdc_hci_subevent_vs_qos_conn_event_report_t *evt;

	code = net_buf_simple_pull_u8(buf);
	if (code != SDC_HCI_SUBEVENT_VS_QOS_CONN_EVENT_REPORT) {
		return false;
	}

	evt = (void *)buf->data;
	printk("QoS reports: (CRC error %d) (rx timeout %d)\n",
			evt->crc_error_count, evt->rx_timeout);

	return true;
}

static int enable_qos_conn_evt_report(void)
{
	int err;
	struct net_buf *buf;

	err = bt_hci_register_vnd_evt_cb(on_vs_evt);
	if (err) {
		printk("Failed registering vendor specific callback (err %d)\n",
		       err);
		return err;
	}

	sdc_hci_cmd_vs_qos_conn_event_report_enable_t *cmd_enable;

	buf = bt_hci_cmd_create(SDC_HCI_OPCODE_CMD_VS_QOS_CONN_EVENT_REPORT_ENABLE,
				sizeof(*cmd_enable));
	if (!buf) {
		printk("Could not allocate command buffer\n");
		return -ENOMEM;
	}

	cmd_enable = net_buf_add(buf, sizeof(*cmd_enable));
	cmd_enable->enable = true;

	err = bt_hci_cmd_send_sync(
		SDC_HCI_OPCODE_CMD_VS_QOS_CONN_EVENT_REPORT_ENABLE, buf, NULL);
	if (err) {
		printk("Could not send command buffer (err %d)\n", err);
		return err;
	}

	printk("QoS connection event reports enabled\n");
	return 0;
}

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	/* Start connectable advertising */
	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Peripheral started, advertising as %s\n", CONFIG_BT_DEVICE_NAME);

	if (enable_qos_conn_evt_report()) {
		printk("Enable QoS connection event reports failed\n");
		return;
	}
}

void main(void)
{
	int err;

	printk("Starting Peripheral Demo\n");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}
}
