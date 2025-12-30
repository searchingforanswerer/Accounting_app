#ifndef ACCOUNTING_MODELS_CHART_TYPE_H_
#define ACCOUNTING_MODELS_CHART_TYPE_H_

namespace accounting {

enum class ChartType {
  kBar,       // 柱状图
  kPie,       // 饼图
  kLine,      // 折线图
  kTable      // 纯数据表格
};

}  // namespace accounting

#endif  // ACCOUNTING_MODELS_CHART_TYPE_H_