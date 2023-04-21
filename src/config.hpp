#ifndef CONFIG_HPP
#define CONFIG_HPP

// Path to BAT dir
static const char* PATH_TO_BATT_CAPACITY = "/sys/class/power_supply/BAT1/capacity";
static const char* PATH_TO_BATT_STATUS = "/sys/class/power_supply/BAT1/status";

// Notifications Message
static const char* NOTIFICATION = "%d%% Battery Remaining. Please plug in the charger.";

// Sleeping time configuration
const int SLEEP_TIME_LONG = 300;
const int SLEEP_TIME_NORMAL = 120;
const int SLEEP_TIME_FAST = 5;

// Battery configuration
const int BATT_CRITICAL = 30;
const int BATT_LOW = 45;

#endif
