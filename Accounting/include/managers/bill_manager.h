#ifndef ACCOUNTING_MANAGERS_BILL_MANAGER_H_
#define ACCOUNTING_MANAGERS_BILL_MANAGER_H_

#include <map>
#include <vector>
#include <memory>
#include <string>
#include "models/bill.h"
#include "models/query_criteria.h"
#include "storage/storage.h"

namespace accounting {

class BillManager {
public:
    BillManager() = default;

    // === 基础操作 ===
    bool AddBill(int user_id, Bill bill);
    bool UpdateBill(int user_id, const Bill& updated_bill);
    bool DeleteBill(int user_id, int bill_id);

    // === 查询接口 ===
    std::vector<Bill> GetBillsByUser(int user_id) const;
    std::vector<Bill> QueryBillsByCriteria(int user_id, const QueryCriteria& criteria) const;

    // === 存储操作 ===
    // LoadFromStorage will restore category pointers using the provided CategoryManager
    bool LoadFromStorage(std::shared_ptr<Storage> storage, const class CategoryManager& category_manager);
    bool SaveToStorage(std::shared_ptr<Storage> storage) const;

private:
    std::map<int, std::vector<Bill>> bills_;   // user_id -> bills
    std::map<int, int> next_bill_id_;          // user_id -> next id
};

}  // namespace accounting

#endif  // ACCOUNTING_MANAGERS_BILL_MANAGER_H_
