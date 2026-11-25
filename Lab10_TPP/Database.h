#pragma once
#include "Models.h"
#include <sqlite_orm/sqlite_orm.h>
#include <string>
inline auto createStorage(const std::string& filename) {
 using namespace sqlite_orm;
 return make_storage(filename,
  make_table("EventSource",make_column("id",&EventSource::id,primary_key().autoincrement()),make_column("name",&EventSource::name,not_null()),make_column("category",&EventSource::category,not_null()),make_column("status",&EventSource::status,not_null())),
  make_table("LogEntry",make_column("id",&LogEntry::id,primary_key().autoincrement()),make_column("source_id",&LogEntry::sourceId,not_null()),make_column("timestamp",&LogEntry::timestamp,not_null()),make_column("level",&LogEntry::level,not_null()),make_column("message",&LogEntry::message,not_null()),make_column("duration_ms",&LogEntry::durationMs,not_null()),foreign_key(&LogEntry::sourceId).references(&EventSource::id)),
 make_table("EntryState",make_column("id",&EntryState::id,primary_key().autoincrement()),make_column("entry_id",&EntryState::entryId,unique()),make_column("status",&EntryState::status),foreign_key(&EntryState::entryId).references(&LogEntry::id)),
 make_table("StateHistory",make_column("id",&StateHistory::id,primary_key().autoincrement()),make_column("entry_id",&StateHistory::entryId),make_column("previous",&StateHistory::previous),make_column("next",&StateHistory::next),foreign_key(&StateHistory::entryId).references(&LogEntry::id)));
}
using Storage = decltype(createStorage(""));
