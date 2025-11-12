#ifndef ACCOUNTING_MODELS_PERIOD_H_
#define ACCOUNTING_MODELS_PERIOD_H_

namespace accounting {

enum class Period {
    kDaily,
    kWeekly,
    kMonthly,
    kYearly,
    kCustom
};

}  // namespace accounting

#endif  // ACCOUNTING_MODELS_PERIOD_H_