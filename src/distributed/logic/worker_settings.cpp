#include "worker_settings.h"

void worker_settings::initialize()
{
    workerId = "worker_" + uuid::v4::UUID::New().String();
}
