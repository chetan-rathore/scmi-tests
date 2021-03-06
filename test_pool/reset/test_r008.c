/** @file
 * Copyright (c) 2020, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#include "val_interface.h"
#include "val_reset.h"

#define TEST_NUM  (SCMI_RESET_TEST_NUM_BASE + 8)
#define TEST_DESC "Reset command invalid flag check             "

#define PARAMETER_SIZE 3

uint32_t reset_query_reset_command_invalid_flag(void)
{
    int32_t  status;
    uint32_t rsp_msg_hdr;
    uint32_t cmd_msg_hdr;
    size_t   param_count;
    size_t   return_value_count;
    uint32_t return_values[MAX_RETURNS_SIZE];
    uint32_t domain_id, num_domains;
    uint32_t parameters[PARAMETER_SIZE];

    if (val_test_initialize(TEST_NUM, TEST_DESC) != VAL_STATUS_PASS)
        return VAL_STATUS_SKIP;

    /* Skip if no domains found*/
    num_domains = val_reset_get_info(NUM_RESET_DOMAINS, 0x0);
    if (num_domains == 0) {
        val_print(VAL_PRINT_ERR, "\n       No reset domains found                      ");
        return VAL_STATUS_SKIP;
    }
    val_print(VAL_PRINT_DEBUG, "\n       NUM DOMAINS                    : %d", num_domains);

    /* Check reset for valid domain with invalid flag*/
    for (domain_id = 0; domain_id < num_domains; domain_id++)
    {
        /* Query with invalid reserve bits */
        val_print(VAL_PRINT_TEST, "\n     RESET DOMAIN ID: %d", domain_id);
        val_print(VAL_PRINT_TEST, "\n     [Check 1] Query reset with invalid reserve bit");

        VAL_INIT_TEST_PARAM(param_count, rsp_msg_hdr, return_value_count, status);
        parameters[param_count++] = domain_id;
        parameters[param_count++] = INVALID_FLAG_VAL;
        parameters[param_count++] = ARCH_COLD_RESET;
        cmd_msg_hdr = val_msg_hdr_create(PROTOCOL_RESET, RESET_PROTOCOL_RESET,
                                         COMMAND_MSG);
        val_send_message(cmd_msg_hdr, param_count, parameters, &rsp_msg_hdr, &status,
                         &return_value_count, return_values);

        if (val_compare_status(status, SCMI_INVALID_PARAMETERS) != VAL_STATUS_PASS)
            return VAL_STATUS_FAIL;

        if (val_compare_msg_hdr(cmd_msg_hdr, rsp_msg_hdr) != VAL_STATUS_PASS)
            return VAL_STATUS_FAIL;

        /* Continue if domain is supported async mode */
        if (val_reset_get_info(RESET_ASYNC_SUPPORT, domain_id) == 1)
            continue;

        /* Query with invalid aync reset flag */
        val_print(VAL_PRINT_TEST, "\n     [Check 2] Query reset with invalid async flag");

        VAL_INIT_TEST_PARAM(param_count, rsp_msg_hdr, return_value_count, status);
        parameters[param_count++] = domain_id;
        parameters[param_count++] = (1 << RESET_FLAG_ASYNC_RESET_BIT) |
                                    (1 << RESET_FLAG_AUTONOMOUS_RESET_BIT);
        parameters[param_count++] = ARCH_COLD_RESET;

        cmd_msg_hdr = val_msg_hdr_create(PROTOCOL_RESET, RESET_PROTOCOL_RESET,
                                         COMMAND_MSG);
        val_send_message(cmd_msg_hdr, param_count, parameters, &rsp_msg_hdr, &status,
                         &return_value_count, return_values);

        if (val_compare_status(status, SCMI_NOT_SUPPORTED) != VAL_STATUS_PASS)
            return VAL_STATUS_FAIL;

        if (val_compare_msg_hdr(cmd_msg_hdr, rsp_msg_hdr) != VAL_STATUS_PASS)
            return VAL_STATUS_FAIL;
    }

    return VAL_STATUS_PASS;
}
