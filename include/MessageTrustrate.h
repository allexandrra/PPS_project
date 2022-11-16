#ifndef NS3_MESSAGE_TRUSTRATE_H
#define NS3_MESSAGE_TRUSTRATE_H

#include "MessageHeader.h"

#define DEFAULT_TRUST 0.5

/* To add :
- After discovering a new host, send your inherent trust to this host.
- Store your trust into a new variable. One upgrade would be to store political,
financial, technological trust and have a function to weight those metrics.
- New metric in routing table : trust to take into account to choose best path.
*/

class MessageTrustrate : public MessageHeader
{
private:
    float trust;

public:
    MessageTrustrate();
    MessageTrustrate(float trust);
    
    float get_trust();

};
#endif