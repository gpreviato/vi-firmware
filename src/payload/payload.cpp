#include <payload/payload.h>
#include <payload/json.h>
#include <payload/protobuf.h>
#include <util/log.h>

namespace payload = openxc::payload;

using openxc::util::log::debug;

openxc_DynamicField openxc::payload::wrapNumber(float value) {
    openxc_DynamicField sabot = {0};
    sabot.has_type = true;
    sabot.type = openxc_DynamicField_Type_NUM;
    sabot.has_numeric_value = true;
    sabot.numeric_value = value;
    return sabot;
}

openxc_DynamicField openxc::payload::wrapString(const char* value) {
    openxc_DynamicField sabot = {0};
    sabot.has_type = true;
    sabot.type = openxc_DynamicField_Type_NUM;
    sabot.has_string_value = true;
    strcpy(sabot.string_value, value);
    return sabot;
}

openxc_DynamicField openxc::payload::wrapBoolean(bool value) {
    openxc_DynamicField sabot = {0};
    sabot.has_type = true;
    sabot.type = openxc_DynamicField_Type_BOOL;
    sabot.has_boolean_value = true;
    sabot.boolean_value = value;
    return sabot;
}

bool openxc::payload::deserialize(uint8_t payload[], size_t length,
        PayloadFormat format, openxc_VehicleMessage* message) {
    bool status = false;
    if(format == PayloadFormat::JSON || format == PayloadFormat::PROTOBUF) {
        // See https://github.com/openxc/vi-firmware/issues/223 - I think we are
        // actually farther along towards that goal than I thought, but the
        // issue is that the Android client sends a JSON command even if the VI
        // is  outputting Protobuf, and the protobuf::deserialize crashes when
        // it receives a non-protobuf payload
        // (https://github.com/openxc/vi-firmware/issues/268)
        status = payload::json::deserialize(payload, length, message);
    } else {
        debug("Invalid payload format: %d", format);
    }
    return status;
}

int openxc::payload::serialize(openxc_VehicleMessage* message,
        uint8_t payload[], size_t length, PayloadFormat format) {
    int serializedLength = 0;
    if(format == PayloadFormat::JSON) {
        serializedLength = payload::json::serialize(message, payload, length);
    } else if(format == PayloadFormat::PROTOBUF) {
        serializedLength = payload::protobuf::serialize(message, payload, length);
    } else {
        debug("Invalid payload format: %d", format);
    }
    return serializedLength;
}
