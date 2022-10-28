#include "../include/MessageTrustrate.h"

MessageTrustrate::MessageTrustrate() : MessageHeader(6) {
  this->trust = DEFAULT_TRUST;
}

MessageTrustrate::MessageTrustrate(float trust) : MessageHeader(6) {
  this->trust = trust;
}

float MessageTrustrate::get_trust() { return this->trust; }
