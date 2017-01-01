#pragma once

#include <string>
#include <vector>
#include <art.h>
#include <sparsepp.h>
#include <store.h>
#include <topster.h>
#include <json.hpp>
#include <field.h>

class Collection {
private:
    Store* store;

    std::string name;

    spp::sparse_hash_map<std::string, field> schema;

    std::vector<std::string> rank_fields;

    // Integer ID used internally for bitmaps - not exposed to the client
    uint32_t seq_id;

    spp::sparse_hash_map<std::string, art_tree*> index_map;

    spp::sparse_hash_map<uint32_t, int64_t> primary_rank_scores;

    spp::sparse_hash_map<uint32_t, int64_t> secondary_rank_scores;

    uint32_t next_seq_id();

    const std::string SEQ_ID_PREFIX = "SQ_";
    const std::string ID_PREFIX = "ID_";
    const std::string META_PREFIX = "MT_";

    const std::string FIELDS_KEY = META_PREFIX + "_fields";

    std::string get_seq_id_key(uint32_t seq_id);
    std::string get_id_key(std::string id);

    static inline std::vector<art_leaf *> next_suggestion(const std::vector<std::vector<art_leaf *>> &token_leaves,
                                                          long long int n);
    void log_leaves(const int cost, const std::string &token, const std::vector<art_leaf *> &leaves) const;

    std::vector<Topster<100>::KV> search(std::string & query, const std::string & field, const int num_typos, const size_t num_results,
                                         std::vector<Topster<100>::KV> & result_kvs, spp::sparse_hash_set<uint64_t> & result_set,
                                         const token_ordering token_order = FREQUENCY, const bool prefix = false);

    void search_candidates(int & token_rank, std::vector<std::vector<art_leaf*>> & token_leaves,
                           std::vector<Topster<100>::KV> & result_kvs, spp::sparse_hash_set<uint64_t> & dedup_seq_ids,
                           size_t & total_results, const size_t & max_results);

    void index_string_field(const std::string &field_name, art_tree *t, const nlohmann::json &document, uint32_t seq_id) const;

    void index_int32_field(const std::string &field_name, art_tree *t, const nlohmann::json &document, uint32_t seq_id) const;

public:
    Collection() = delete;
    Collection(const std::string & state_dir_path, const std::string & name, const std::vector<field> & search_fields,
               const std::vector<std::string> rank_fields);
    ~Collection();
    std::string add(std::string json_str);
    std::vector<nlohmann::json> search(std::string query, const std::vector<std::string> fields, const int num_typos,
                                       const size_t num_results, const token_ordering token_order = FREQUENCY,
                                       const bool prefix = false);
    void remove(std::string id);
    void score_results(Topster<100> &topster, const int & token_rank, const std::vector<art_leaf *> &query_suggestion,
                       const uint32_t *result_ids, const size_t result_size) const;

    enum {MAX_SEARCH_TOKENS = 20};
    enum {MAX_RESULTS = 100};
};
