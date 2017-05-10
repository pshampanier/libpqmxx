/**
 * Copyright (c) 2016 Philippe FERDINAND
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/

#include "postgres-connection.h"
#include "postgres-exceptions.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>

namespace libpqmxx {

  static auto error_code_map = std::map<const char *, error_code, std::function<bool (const char *a, const char *b)>> {
    [](const char *a, const char *b) {
      return std::strcmp(a, b) < 0;
    }
  } = {
    { "00000", error_code::successful_completion },
    { "01000", error_code::warning },
    { "0100C", error_code::dynamic_result_sets_returned },
    { "01008", error_code::implicit_zero_bit_padding },
    { "01003", error_code::null_value_eliminated_in_set_function },
    { "01007", error_code::privilege_not_granted },
    { "01006", error_code::privilege_not_revoked },
    { "01004", error_code::string_data_right_truncation },
    { "01P01", error_code::deprecated_feature },
    { "02000", error_code::no_data },
    { "02001", error_code::no_additional_dynamic_result_sets_returned },
    { "03000", error_code::sql_statement_not_yet_complete },
    { "08000", error_code::connection_exception },
    { "08003", error_code::connection_does_not_exist },
    { "08006", error_code::connection_failure },
    { "08001", error_code::sqlclient_unable_to_establish_sqlconnection },
    { "08004", error_code::sqlserver_rejected_establishment_of_sqlconnection },
    { "08007", error_code::transaction_resolution_unknown },
    { "08P01", error_code::protocol_violation },
    { "09000", error_code::triggered_action_exception },
    { "0A000", error_code::feature_not_supported },
    { "0B000", error_code::invalid_transaction_initiation },
    { "0F000", error_code::locator_exception },
    { "0F001", error_code::invalid_locator_specification },
    { "0L000", error_code::invalid_grantor },
    { "0LP01", error_code::invalid_grant_operation },
    { "0P000", error_code::invalid_role_specification },
    { "0Z000", error_code::diagnostics_exception },
    { "0Z002", error_code::stacked_diagnostics_accessed_without_active_handler },
    { "20000", error_code::case_not_found },
    { "21000", error_code::cardinality_violation },
    { "22000", error_code::data_exception },
    { "2202E", error_code::array_subscript_error },
    { "22021", error_code::character_not_in_repertoire },
    { "22008", error_code::datetime_field_overflow },
    { "22012", error_code::division_by_zero },
    { "22005", error_code::error_in_assignment },
    { "2200B", error_code::escape_character_conflict },
    { "22022", error_code::indicator_overflow },
    { "22015", error_code::interval_field_overflow },
    { "2201E", error_code::invalid_argument_for_logarithm },
    { "22014", error_code::invalid_argument_for_ntile_function },
    { "22016", error_code::invalid_argument_for_nth_value_function },
    { "2201F", error_code::invalid_argument_for_power_function },
    { "2201G", error_code::invalid_argument_for_width_bucket_function },
    { "22018", error_code::invalid_character_value_for_cast },
    { "22007", error_code::invalid_datetime_format },
    { "22019", error_code::invalid_escape_character },
    { "2200D", error_code::invalid_escape_octet },
    { "22025", error_code::invalid_escape_sequence },
    { "22P06", error_code::nonstandard_use_of_escape_character },
    { "22010", error_code::invalid_indicator_parameter_value },
    { "22023", error_code::invalid_parameter_value },
    { "2201B", error_code::invalid_regular_expression },
    { "2201W", error_code::invalid_row_count_in_limit_clause },
    { "2201X", error_code::invalid_row_count_in_result_offset_clause },
    { "2202H", error_code::invalid_tablesample_argument },
    { "2202G", error_code::invalid_tablesample_repeat },
    { "22009", error_code::invalid_time_zone_displacement_value },
    { "2200C", error_code::invalid_use_of_escape_character },
    { "2200G", error_code::most_specific_type_mismatch },
    { "22004", error_code::null_value_not_allowed },
    { "22002", error_code::null_value_no_indicator_parameter },
    { "22003", error_code::numeric_value_out_of_range },
    { "22026", error_code::string_data_length_mismatch },
    { "22001", error_code::string_data_right_truncation },
    { "22011", error_code::substring_error },
    { "22027", error_code::trim_error },
    { "22024", error_code::unterminated_c_string },
    { "2200F", error_code::zero_length_character_string },
    { "22P01", error_code::floating_point_exception },
    { "22P02", error_code::invalid_text_representation },
    { "22P03", error_code::invalid_binary_representation },
    { "22P04", error_code::bad_copy_file_format },
    { "22P05", error_code::untranslatable_character },
    { "2200L", error_code::not_an_xml_document },
    { "2200M", error_code::invalid_xml_document },
    { "2200N", error_code::invalid_xml_content },
    { "2200S", error_code::invalid_xml_comment },
    { "2200T", error_code::invalid_xml_processing_instruction },
    { "23000", error_code::integrity_constraint_violation },
    { "23001", error_code::restrict_violation },
    { "23502", error_code::not_null_violation },
    { "23503", error_code::foreign_key_violation },
    { "23505", error_code::unique_violation },
    { "23514", error_code::check_violation },
    { "23P01", error_code::exclusion_violation },
    { "24000", error_code::invalid_cursor_state },
    { "25000", error_code::invalid_transaction_state },
    { "25001", error_code::active_sql_transaction },
    { "25002", error_code::branch_transaction_already_active },
    { "25008", error_code::held_cursor_requires_same_isolation_level },
    { "25003", error_code::inappropriate_access_mode_for_branch_transaction },
    { "25004", error_code::inappropriate_isolation_level_for_branch_transaction },
    { "25005", error_code::no_active_sql_transaction_for_branch_transaction },
    { "25006", error_code::read_only_sql_transaction },
    { "25007", error_code::schema_and_data_statement_mixing_not_supported },
    { "25P01", error_code::no_active_sql_transaction },
    { "25P02", error_code::in_failed_sql_transaction },
    { "25P03", error_code::idle_in_transaction_session_timeout },
    { "26000", error_code::invalid_sql_statement_name },
    { "27000", error_code::triggered_data_change_violation },
    { "28000", error_code::invalid_authorization_specification },
    { "28P01", error_code::invalid_password },
    { "2B000", error_code::dependent_privilege_descriptors_still_exist },
    { "2BP01", error_code::dependent_objects_still_exist },
    { "2D000", error_code::invalid_transaction_termination },
    { "2F000", error_code::sql_routine_exception },
    { "2F005", error_code::function_executed_no_return_statement },
    { "2F002", error_code::modifying_sql_data_not_permitted },
    { "2F003", error_code::prohibited_sql_statement_attempted },
    { "2F004", error_code::reading_sql_data_not_permitted },
    { "34000", error_code::invalid_cursor_name },
    { "38000", error_code::external_routine_exception },
    { "38001", error_code::containing_sql_not_permitted },
    { "38002", error_code::modifying_sql_data_not_permitted },
    { "38003", error_code::prohibited_sql_statement_attempted },
    { "38004", error_code::reading_sql_data_not_permitted },
    { "39000", error_code::external_routine_invocation_exception },
    { "39001", error_code::invalid_sqlstate_returned },
    { "39004", error_code::null_value_not_allowed },
    { "39P01", error_code::trigger_protocol_violated },
    { "39P02", error_code::srf_protocol_violated },
    { "39P03", error_code::event_trigger_protocol_violated },
    { "3B000", error_code::savepoint_exception },
    { "3B001", error_code::invalid_savepoint_specification },
    { "3D000", error_code::invalid_catalog_name },
    { "3F000", error_code::invalid_schema_name },
    { "40000", error_code::transaction_rollback },
    { "40002", error_code::transaction_integrity_constraint_violation },
    { "40001", error_code::serialization_failure },
    { "40003", error_code::statement_completion_unknown },
    { "40P01", error_code::deadlock_detected },
    { "42000", error_code::syntax_error_or_access_rule_violation },
    { "42601", error_code::syntax_error },
    { "42501", error_code::insufficient_privilege },
    { "42846", error_code::cannot_coerce },
    { "42803", error_code::grouping_error },
    { "42P20", error_code::windowing_error },
    { "42P19", error_code::invalid_recursion },
    { "42830", error_code::invalid_foreign_key },
    { "42602", error_code::invalid_name },
    { "42622", error_code::name_too_long },
    { "42939", error_code::reserved_name },
    { "42804", error_code::datatype_mismatch },
    { "42P18", error_code::indeterminate_datatype },
    { "42P21", error_code::collation_mismatch },
    { "42P22", error_code::indeterminate_collation },
    { "42809", error_code::wrong_object_type },
    { "42703", error_code::undefined_column },
    { "42883", error_code::undefined_function },
    { "42P01", error_code::undefined_table },
    { "42P02", error_code::undefined_parameter },
    { "42704", error_code::undefined_object },
    { "42701", error_code::duplicate_column },
    { "42P03", error_code::duplicate_cursor },
    { "42P04", error_code::duplicate_database },
    { "42723", error_code::duplicate_function },
    { "42P05", error_code::duplicate_prepared_statement },
    { "42P06", error_code::duplicate_schema },
    { "42P07", error_code::duplicate_table },
    { "42712", error_code::duplicate_alias },
    { "42710", error_code::duplicate_object },
    { "42702", error_code::ambiguous_column },
    { "42725", error_code::ambiguous_function },
    { "42P08", error_code::ambiguous_parameter },
    { "42P09", error_code::ambiguous_alias },
    { "42P10", error_code::invalid_column_reference },
    { "42611", error_code::invalid_column_definition },
    { "42P11", error_code::invalid_cursor_definition },
    { "42P12", error_code::invalid_database_definition },
    { "42P13", error_code::invalid_function_definition },
    { "42P14", error_code::invalid_prepared_statement_definition },
    { "42P15", error_code::invalid_schema_definition },
    { "42P16", error_code::invalid_table_definition },
    { "42P17", error_code::invalid_object_definition },
    { "44000", error_code::with_check_option_violation },
    { "53000", error_code::insufficient_resources },
    { "53100", error_code::disk_full },
    { "53200", error_code::out_of_memory },
    { "53300", error_code::too_many_connections },
    { "53400", error_code::configuration_limit_exceeded },
    { "54000", error_code::program_limit_exceeded },
    { "54001", error_code::statement_too_complex },
    { "54011", error_code::too_many_columns },
    { "54023", error_code::too_many_arguments },
    { "55000", error_code::object_not_in_prerequisite_state },
    { "55006", error_code::object_in_use },
    { "55P02", error_code::cant_change_runtime_param },
    { "55P03", error_code::lock_not_available },
    { "57000", error_code::operator_intervention },
    { "57014", error_code::query_canceled },
    { "57P01", error_code::admin_shutdown },
    { "57P02", error_code::crash_shutdown },
    { "57P03", error_code::cannot_connect_now },
    { "57P04", error_code::database_dropped },
    { "58000", error_code::system_error },
    { "58030", error_code::io_error },
    { "58P01", error_code::undefined_file },
    { "58P02", error_code::duplicate_file },
    { "72000", error_code::snapshot_too_old },
    { "F0000", error_code::config_file_error },
    { "F0001", error_code::lock_file_exists },
    { "HV000", error_code::fdw_error },
    { "HV005", error_code::fdw_column_name_not_found },
    { "HV002", error_code::fdw_dynamic_parameter_value_needed },
    { "HV010", error_code::fdw_function_sequence_error },
    { "HV021", error_code::fdw_inconsistent_descriptor_information },
    { "HV024", error_code::fdw_invalid_attribute_value },
    { "HV007", error_code::fdw_invalid_column_name },
    { "HV008", error_code::fdw_invalid_column_number },
    { "HV004", error_code::fdw_invalid_data_type },
    { "HV006", error_code::fdw_invalid_data_type_descriptors },
    { "HV091", error_code::fdw_invalid_descriptor_field_identifier },
    { "HV00B", error_code::fdw_invalid_handle },
    { "HV00C", error_code::fdw_invalid_option_index },
    { "HV00D", error_code::fdw_invalid_option_name },
    { "HV090", error_code::fdw_invalid_string_length_or_buffer_length },
    { "HV00A", error_code::fdw_invalid_string_format },
    { "HV009", error_code::fdw_invalid_use_of_null_pointer },
    { "HV014", error_code::fdw_too_many_handles },
    { "HV001", error_code::fdw_out_of_memory },
    { "HV00P", error_code::fdw_no_schemas },
    { "HV00J", error_code::fdw_option_name_not_found },
    { "HV00K", error_code::fdw_reply_handle },
    { "HV00Q", error_code::fdw_schema_not_found },
    { "HV00R", error_code::fdw_table_not_found },
    { "HV00L", error_code::fdw_unable_to_create_execution },
    { "HV00M", error_code::fdw_unable_to_create_reply },
    { "HV00N", error_code::fdw_unable_to_establish_connection },
    { "P0000", error_code::plpgsql_error },
    { "P0001", error_code::raise_exception },
    { "P0002", error_code::no_data_found },
    { "P0003", error_code::too_many_rows },
    { "P0004", error_code::assert_failure },
    { "XX000", error_code::internal_error },
    { "XX001", error_code::data_corrupted },
    { "XX002", error_code::index_corrupted }
  };

  /**
   * A reference to the connection.
   *
   * In asynchonous mode, it's mandatory to create the connection on the heap
   * using a shared_ptr while on blocking mode it's practical to be able to
   * create the Connection on the stack. The ConnectionRef make both possible
   * by keeping a reference to connection in both cases but also keeping a
   * shared_ptr when the connection is asynchronous. This way the owner of
   * a ConnectionRef can extend the scope of an asynchronous Connection to its
   * own scope and acommodate with blocking Connection created on the stack.
   **/
  struct ConnectionRef {

    Connection &conn;
    std::shared_ptr<Connection> conn_ptr;

    ConnectionRef(Connection &conn)
    : conn(conn) {
      if (conn.async()) {
        conn_ptr = conn.shared_from_this();
      }
    }

    Connection & operator * () const noexcept {
      return conn;
    }

  };

  /**************************************************************************
   * User API handler for asynchronous results.
   **************************************************************************/
  template <typename T>
  struct ResultHandler {
    
    std::shared_ptr<ResultHandle> handle;             /**< Keep the object alive until the callback exec. **/
    std::function<void (T &arg)>  callback = nullptr; /**< The registred callback. **/
    
    void reset() noexcept {
      handle = nullptr;
      callback = nullptr;
    }
    
  };

  /**************************************************************************
   * Handle to the native PostgreSQL result.
   **************************************************************************/
  class ResultHandle : public std::enable_shared_from_this<ResultHandle> {
  public:

    PGresult                   *pgresult = nullptr;              /** libpq result        */
    ExecStatusType              status   = PGRES_EMPTY_QUERY;    /** Current status      */
    int                         rowNum;                          /** Current row number. */
    int                         rowCount;
    ConnectionRef               conn;
    std::exception_ptr          lastException;
    bool                        asyncIterator = false; /** true if once() or each() have been called */
    bool                        singleRowMode = false;

    ResultHandler<Row>                 each;
    ResultHandler<Result>              done;
    ResultHandler<std::exception_ptr>  error;
    ResultHandler<Connection>          always;

    // -------------------------------------------------------------------------
    // Constructor
    // -------------------------------------------------------------------------
    ResultHandle(Connection &conn)
    : conn(conn) {
      singleRowMode = conn.singleRowMode_;
    }

    // -------------------------------------------------------------------------
    // Desctuctor
    // -------------------------------------------------------------------------
    ~ResultHandle() {
      if (pgresult) {
        PQclear(pgresult);
        if (!(*conn).async_) {
          pgresult = PQgetResult(*conn);
          assert(pgresult == nullptr);
        }
      }
    }
    
    // -------------------------------------------------------------------------
    // Free all resources owned by this result.
    // -------------------------------------------------------------------------
    void clear() {
      each.reset();
      done.reset();
      error.reset();
      if (pgresult) {
        PQclear(pgresult);
        if (!(*conn).async_) {
          pgresult = PQgetResult(*conn);
          assert(pgresult == nullptr);
        }
        else {
          pgresult = nullptr;
        }
      }
      if (always.callback) {
        always.callback((*conn));
        always.reset();
      }
    }

    // -------------------------------------------------------------------------
    // Check if the result may have more rows.
    // -------------------------------------------------------------------------
    bool hasNext() const noexcept {
      if (singleRowMode) {
        return status == PGRES_SINGLE_TUPLE;
      }
      else {
        return rowNum < rowCount;
      }
    }

    // -------------------------------------------------------------------------
    // Position of the iteration in pgresult
    // -----------------------------------------------------------------------
    int offset() const noexcept {
      return singleRowMode ? 0 : rowNum - 1;
    }

    std::exception_ptr lastError() {

      char *state = PQresultErrorField(pgresult, PG_DIAG_SQLSTATE);
      auto code = error_code::unknown;
      if (state) {
        auto it = error_code_map.find(state);
        if (it != error_code_map.end()) {
          code = (*it).second;
        }
      }

      std::string lastError = (*conn).lastPostgresError();      
      if (!(*conn).isConnected()) {
        return std::make_exception_ptr(connection_error(lastError, code));
      }
      else {
        switch (code) {
          case error_code::connection_does_not_exist:                         /** 08003 **/
          case error_code::connection_failure:                                /** 08006 **/
          case error_code::sqlclient_unable_to_establish_sqlconnection:       /** 08001 **/
          case error_code::sqlserver_rejected_establishment_of_sqlconnection: /** 08004 **/
          case error_code::transaction_resolution_unknown:                    /** 08007 **/
          case error_code::protocol_violation:                                /** 08P01 **/
            return std::make_exception_ptr(connection_error(lastError, code));

          default:
            return std::make_exception_ptr(libpqmxx::error(lastError, code));
        }
      }

    }

    void setLastError(std::exception_ptr eptr) {
      if ((*conn).async_) {
        lastException = eptr;
        if (error.callback) {
          error.callback(eptr);
          clear();
        }
      }
      else {
        std::rethrow_exception(eptr);
      }
      
    }

    void attach(PGresult *pgr) {
      pgresult = pgr;
      if (pgresult == nullptr
          && (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK)) {
        // No more statements
        status = PGRES_EMPTY_QUERY;
      }
      else {
        status = PQresultStatus(pgresult);
        switch (status) {
          case PGRES_SINGLE_TUPLE:
            assert(PQntuples(pgresult) == 1);
            rowNum++;
            break;

          case PGRES_TUPLES_OK:
            rowCount = PQntuples(pgresult);
            if (singleRowMode) {
              // The SELECT statement did not return any row or after the last row,
              // a zero-row object with status PGRES_TUPLES_OK is returned; this is
              // the signal that no more rows are expected.
              assert(rowCount == 0);
            }
            else {
              rowNum = 1;
            }
            if (!(*conn).async_) {
              releaseConnection();
            }
            break;

          case PGRES_BAD_RESPONSE:
          case PGRES_FATAL_ERROR: {
            if (!(*conn).async_) {
              releaseConnection();
            }
            setLastError(lastError());
            break;
          }

          case PGRES_COMMAND_OK:
            if (!(*conn).async_) {
              releaseConnection();
            }
            break;
            
          case PGRES_EMPTY_QUERY:
            releaseConnection();
            throw libpqmxx::error("Empty query.", error_code::syntax_error);
            break;

          default:
            assert(false);  // LCOV_EXCL_LINE
            break;
        }
      }
    }

    // -----------------------------------------------------------------------
    // Get the first result from the server.
    // -----------------------------------------------------------------------
    void first() {
      assert(pgresult == nullptr);
      rowNum = 0;
      next();
    }

    // -----------------------------------------------------------------------
    // Read the next result from the server.
    // -----------------------------------------------------------------------
    void next() {
      if (pgresult) {
        // assert(status == PGRES_SINGLE_TUPLE || status == PGRES_COMMAND_OK);
        PQclear(pgresult);
        pgresult = nullptr;
      }
      attach(PQgetResult(*conn));
    }

    void consumeInput() {
      int success = PQconsumeInput(*conn);
      if (!success) {
        setLastError(lastError());
      }
      else if (!PQisBusy(*conn)) {
        try {
          Result result(*this);
          if (singleRowMode) {
            do {
              next();
              if (each.callback) {
                each.callback(result);
              }
            } while (!PQisBusy(*conn) && hasNext());
          }
          else {
            // Result is ready
            next();
            if (!lastException) {
              ResultHandler<Result> handler;
              std::swap(done, handler);
              handler.callback(result);
              if (!done.callback) {
                // No more result expected. If more result were expected the
                // handler callback should have called done() to register a
                // handler for the next result.
                releaseConnection();
                clear();
              }
            }
          }
        }
        catch (...) {
          setLastError(std::current_exception());
        }
      }
    }

    void releaseConnection() const noexcept {
      
      auto lastResult = (*conn).lastResult_.lock();
      if (!(*conn).async_ || lastResult.get() == this) {
        switch (status) {
          case PGRES_FATAL_ERROR:
          case PGRES_BAD_RESPONSE: {
            PGresult *pgres = PQgetResult(*conn);
            if (pgres) {
              // After a connection broken, this is expected to get an error here.
              assert(!(*conn).isConnected());
              PQclear(pgres);
            }
            break;
          }
            
          case PGRES_COMMAND_OK:
          case PGRES_NONFATAL_ERROR:
          case PGRES_TUPLES_OK: {
            PGresult *pgres = PQgetResult(*conn);
            if (pgres) {
              assert(pgres == nullptr);
              PQclear(pgres);
            }
            break;
          }

          case PGRES_EMPTY_QUERY:
          case PGRES_SINGLE_TUPLE:
            // Not expected.
            assert(true); // LCOV_EXCL_LINE
            break;        // LCOV_EXCL_LINE

          case PGRES_COPY_OUT:			/* Copy Out data transfer in progress */
          case PGRES_COPY_IN:				/* Copy In data transfer in progress */
          case PGRES_COPY_BOTH:			/* Copy In/Out data transfer in progress */
            // Not yet supported.
            assert(true); // LCOV_EXCL_LINE
            break;        // LCOV_EXCL_LINE

        }
      }
    }
    
    /**
     * Check if the native PostgreSQL result is set.
     **/
    operator bool () const {
      return pgresult != nullptr;
    }

    /**
     * Cast to the native PostgreSQL result.
     **/
    operator const PGresult *() const {
      return pgresult;
    }
  };

  void consumeInput(ResultHandle *rh) {
    assert(rh);
    rh->consumeInput();
  }

  void attach(ResultHandle *rh, PGresult *pgr) {
    rh->attach(pgr);
  }

  void releaseConnection(ResultHandle *rh) {
    rh->releaseConnection();
  }

  void setLastError(ResultHandle *rh, std::exception_ptr eptr) {
    rh->setLastError(eptr);
  }

#ifdef NDEBUG

  #define assert_oid(expected, actual) ((void)0)

#else
  /* LCOV_EXCL_START */
  void assert_oid(int expected, int actual) {
    const char *_expected = nullptr;
    if (expected != UNKNOWNOID && expected != actual) {
      switch (expected) {
        case BOOLOID: _expected = "std::vector<uint_8>"; break;
        case BYTEAOID: _expected = "char"; break;
        case CHAROID: _expected = "std::string"; break;
        case NAMEOID: _expected = "std::string"; break;
        case INT8OID: _expected = "int64_t"; break;
        case INT2OID: _expected = "int16_t"; break;
        case INT4OID: _expected = "int32_t"; break;
        case TEXTOID: _expected = "std::string"; break;
        case FLOAT4OID: _expected = "float"; break;
        case FLOAT8OID: _expected = "double"; break;
        case BPCHAROID: _expected = "std::string"; break;
        case VARCHAROID: _expected = "std::string"; break;
        case DATEOID: _expected = "libpqmxx::date_t"; break;
        case TIMEOID: _expected = "libpqmxx::time_t"; break;
        case TIMESTAMPOID: _expected = "libpqmxx::timestamp_t"; break;
        case TIMESTAMPTZOID: _expected = "libpqmxx::timestamptz_t"; break;
        case INTERVALOID: _expected = "libpqmxx::interval_t"; break;
        case TIMETZOID: _expected = "libpqmxx::timetz_t"; break;
        default:
          assert(false); // unsupported type. try std::string
      }

      std::cerr << "Unexpected C++ type. Please use as<" << _expected
        << ">(int column)" << std::endl;
      assert(false);
    }
  }
  /* LCOV_EXCL_STOP */

#endif

  // -------------------------------------------------------------------------
  // Reading a value from a PGresult
  // -------------------------------------------------------------------------
  template <typename T>
  T read(const PGresult *pgresult, int row, int column) {
    char *buf = PQgetvalue(pgresult, row, column);
    return read<T>(&buf);
  }

  template <typename T>
  T read(const PGresult *pgresult, int oid, int row, int column, T defVal) {
    assert(pgresult != nullptr);
    assert_oid(PQftype(pgresult, column), oid);
    return PQgetisnull(pgresult, row, column) ? defVal : read<T>(pgresult, row, column);
  }

  template<typename T>
  std::vector<array_item<T>> readArray(const PGresult *pgresult, int oid, int row, int column, T defVal) {
    std::vector<array_item<T>> array;

    if (!PQgetisnull(pgresult, row, column)) {
      // The data should look like this:
      //
      // struct pg_array {
      //   int32_t ndim; /* Number of dimensions */
      //   int32_t ign;  /* offset for data, removed by libpq */
      //   Oid elemtype; /* type of element in the array */
      //
      //   /* First dimension */
      //   int32_t size;  /* Number of elements */
      //   int32_t index; /* Index of first element */
      //   T first_value; /* Beginning of the data */
      // }
      char *buf = PQgetvalue(pgresult, row, column);
      int32_t ndim = read<int32_t>(&buf);
      read<int32_t>(&buf); // skip
      int32_t elemType = read<int32_t>(&buf);
      assert_oid(oid, elemType);
      assert(ndim == 1); // only array of 1 dimmension are supported so far.

      // First dimension
      int32_t size = read<int32_t>(&buf);
      read<int32_t>(&buf); // skip the index of first element.

      int32_t elemSize;
      array.reserve(size);
      for (int32_t i=0; i < size; i++) {
        elemSize = read<int32_t>(&buf);
        array_item<T> elem;
        if (elemSize == -1) {
          // null value
          elem.isNull = true;
          elem.value = defVal;
        }
        else {
          elem.isNull = false;
          elem.value = read<T>(&buf, elemSize);
        }

        array.push_back(std::move(elem));
      }
    }

    return array;
  }

  // -------------------------------------------------------------------------
  // Row contructor
  // -------------------------------------------------------------------------
  Row::Row(Result &result)
  : result_(result) {
  }

  // -------------------------------------------------------------------------
  // Row number.
  // -------------------------------------------------------------------------
  int Row::num() const noexcept {
    return result_.handle_->rowNum;
  }

  // -------------------------------------------------------------------------
  // Current row to read in the server response.
  // -------------------------------------------------------------------------
  int Row::offset() const noexcept {
    return result_.handle_->offset();
  }

  // -------------------------------------------------------------------------
  // Tests a column for a null value.
  // -------------------------------------------------------------------------
  bool Row::isNull(int column) const {
    assert(*result_.handle_ != nullptr);
    return PQgetisnull(result_, offset(), column) == 1;
  }
        
  // -------------------------------------------------------------------------
  // Length of a column value.
  // -------------------------------------------------------------------------
  size_t Row::length(int column) const {
    assert(*result_.handle_ != nullptr);
    return PQgetlength(result_, offset(), column);
  }

  // -------------------------------------------------------------------------
  // Get a column name.
  // -------------------------------------------------------------------------
  const char *Row::columnName(int column) const {
    assert(*result_.handle_);
    const char *res = PQfname(result_, column);
    assert(res);
    return res;
  }

  template<>
  bool Row::as<bool>(int column) const {
    return read<bool>(result_, BOOLOID, offset(), column, false);
  }

  template<>
  int16_t Row::as<int16_t>(int column) const {
    return read<int16_t>(result_, INT2OID, offset(), column, 0);
  }

  template<>
  int32_t Row::as<int32_t>(int column) const {
    return read<int32_t>(result_, INT4OID, offset(), column, 0);
  }

  template<>
  int64_t Row::as<int64_t>(int column) const {
    return read<int64_t>(result_, INT8OID, offset(), column, 0);
  }

  template<>
  float Row::as<float>(int column) const {
    return read<float>(result_, FLOAT4OID, offset(), column, 0.f);
  }

  template<>
  double Row::as<double>(int column) const {
    return read<double>(result_, FLOAT8OID, offset(), column, 0.);
  }

  template<>
  std::string Row::as<std::string>(int column) const {
    assert(*result_.handle_);
    if (PQgetisnull(result_, offset(), column)) {
      return std::string();
    }
    int length = PQgetlength(result_, offset(), column);
    char *buf  = PQgetvalue(result_, offset(), column);
    return read<std::string>(&buf, length);
  }
        
  template<>
  const char *Row::as<const char *>(int column) const {
    assert(*result_.handle_);
    if (PQgetisnull(result_, offset(), column)) {
      return nullptr;
    }
    return PQgetvalue(result_, offset(), column);
  }

  // -------------------------------------------------------------------------
  // "char"
  // -------------------------------------------------------------------------
  template<>
  char Row::as<char>(int column) const {
    assert(*result_.handle_);
    if (PQgetisnull(result_, offset(), column)) {
      return '\0';
    }
    assert(PQgetlength(result_, offset(), column) == 1);
    return *PQgetvalue(result_, offset(), column);
  }

  // -------------------------------------------------------------------------
  // bytea
  // -------------------------------------------------------------------------
  template<>
  std::vector<uint8_t> Row::as<std::vector<uint8_t>>(int column) const {
    assert(*result_.handle_);
    assert_oid(PQftype(result_, column), BYTEAOID);
    int length = PQgetlength(result_, offset(), column);
    uint8_t *data = reinterpret_cast<uint8_t *>(PQgetvalue(result_, offset(), column));
    return std::vector<uint8_t>(data, data + length);
  }

  template<>
  date_t Row::as<date_t>(int column) const {
    return read<date_t>(result_, DATEOID, offset(), column, date_t { 0 });
  }

  template<>
  timestamptz_t Row::as<timestamptz_t>(int column) const {
    return read<timestamptz_t>(result_, TIMESTAMPTZOID, offset(), column, timestamptz_t { 0 });
  }

  template<>
  timestamp_t Row::as<timestamp_t>(int column) const {
    return read<timestamp_t>(result_, TIMESTAMPOID, offset(), column, timestamp_t { 0 });
  }

  template<>
  timetz_t Row::as<timetz_t>(int column) const {
    return read<timetz_t>(result_, TIMETZOID, offset(), column, timetz_t { 0, 0 });
  }

  template<>
  time_t Row::as<time_t>(int column) const {
    return read<time_t>(result_, TIMEOID, offset(), column, time_t { 0 });
  }

  template<>
  interval_t Row::as<interval_t>(int column) const {
    return read<interval_t>(result_, INTERVALOID, offset(), column, interval_t { 0, 0, 0 });
  }

  // -------------------------------------------------------------------------
  // Arrays
  // -------------------------------------------------------------------------

  template<>
  std::vector<array_item<bool>> Row::asArray(int column) const {
    return readArray<bool>(result_, BOOLOID, offset(), column, 0);
  }

  template<>
  std::vector<array_item<int16_t>> Row::asArray<int16_t>(int column) const {
    return readArray<int16_t>(result_, INT2OID, offset(), column, 0);
  }

  template<>
  std::vector<array_item<int32_t>> Row::asArray<int32_t>(int column) const {
    return readArray<int32_t>(result_, INT4OID, offset(), column, 0);
  }

  template<>
  std::vector<array_item<int64_t>> Row::asArray<int64_t>(int column) const {
    return readArray<int64_t>(result_, INT8OID, offset(), column, 0);
  }

  template<>
  std::vector<array_item<float>> Row::asArray<float>(int column) const {
    return readArray<float>(result_, FLOAT4OID, offset(), column, 0.f);
  }

  template<>
  std::vector<array_item<double>> Row::asArray<double>(int column) const {
    return readArray<double>(result_, FLOAT8OID, offset(), column, 0.);
  }

  template<>
  std::vector<array_item<date_t>> Row::asArray<date_t>(int column) const {
    return readArray<date_t>(result_, DATEOID, offset(), column, date_t { 0 });
  }

  template<>
  std::vector<array_item<timestamptz_t>> Row::asArray<timestamptz_t>(int column) const {
    return readArray<timestamptz_t>(result_, TIMESTAMPTZOID, offset(), column, timestamptz_t { 0 });
  }

  template<>
  std::vector<array_item<timestamp_t>> Row::asArray<timestamp_t>(int column) const {
    return readArray<timestamp_t>(result_, TIMESTAMPOID, offset(), column, timestamp_t { 0 });
  }

  template<>
  std::vector<array_item<timetz_t>> Row::asArray<timetz_t>(int column) const {
    return readArray<timetz_t>(result_, TIMETZOID, offset(), column, timetz_t { 0, 0 });
  }

  template<>
  std::vector<array_item<time_t>> Row::asArray<time_t>(int column) const {
    return readArray<time_t>(result_, TIMEOID, offset(), column, time_t { 0 });
  }

  template<>
  std::vector<array_item<interval_t>> Row::asArray<interval_t>(int column) const {
    return readArray<interval_t>(result_, INTERVALOID, offset(), column, interval_t { 0, 0, 0 });
  }

  template<>
  std::vector<array_item<std::string>> Row::asArray<std::string>(int column) const {
    return readArray<std::string>(result_, UNKNOWNOID, offset(), column, std::string());
  }

  // -------------------------------------------------------------------------
  // Result contructor
  // -------------------------------------------------------------------------
  Result::Result(Connection &conn)
    : Row(*this), begin_(*this), end_(*this) {
    handle_ = std::make_shared<ResultHandle>(conn);
  }

  Result::Result(ResultHandle &handle)
  : Row(*this), begin_(*this), end_(*this) {
    handle_ = handle.shared_from_this();
  }

  // -------------------------------------------------------------------------
  // Copy constructor.
  // -------------------------------------------------------------------------
  Result::Result(const Result &other)
    : Result(*other.handle_.get()) {
  }

  // -------------------------------------------------------------------------
  // Cast to the native PostgreSQL result.
  // -------------------------------------------------------------------------
  Result::operator const PGresult *() const {
    return handle_->pgresult;
  }

  // -------------------------------------------------------------------------
  // Get the first result from the server.
  // -------------------------------------------------------------------------
  void Result::first() {
    handle_->first();
  }

  // -------------------------------------------------------------------------
  // Get the next result from the server.
  // -------------------------------------------------------------------------
  void Result::next() {
    if (handle_->singleRowMode) {
      handle_->next();
    }
    else {
      handle_->rowNum++;
    }
  }

  // -------------------------------------------------------------------------
  // First row of the result
  // -------------------------------------------------------------------------
  Result::iterator Result::begin() {
    return Result::iterator(&begin_);
  }

  // -------------------------------------------------------------------------
  // Last row of the result
  // -------------------------------------------------------------------------
  Result::iterator Result::end() {
    // If there is no result available then end() = begin()
    if (handle_->singleRowMode) {
      return Result::iterator(handle_->status == PGRES_SINGLE_TUPLE ? &end_ : &begin_);
    }
    else {
      return Result::iterator(handle_->rowNum <= handle_->rowCount? &end_ : &begin_);
    }
  }

  // -------------------------------------------------------------------------
  // Next row of the result
  // -------------------------------------------------------------------------
  Result::iterator Result::iterator::operator ++() {
    ptr_->result_.next();
    if (ptr_->result_.handle_->singleRowMode) {
      if (ptr_->result_.handle_->status != PGRES_SINGLE_TUPLE) {
        // We've reached the end
        assert(ptr_->result_.handle_->status == PGRES_TUPLES_OK);
        ptr_ = &ptr_->result_.end_;
      }
    }
    else if (ptr_->result_.handle_->rowNum > ptr_->result_.handle_->rowCount) {
      // We've reached the end
      ptr_ = &ptr_->result_.end_;
    }
    return iterator (ptr_);
  }

  // -------------------------------------------------------------------------
  // Number of rows affected by the SQL command
  // -------------------------------------------------------------------------
  uint64_t Result::count() const noexcept {
    assert(handle_);
    const char *count = PQcmdTuples(handle_->pgresult);
    char *end;
    return std::strtoull(count, &end, 10);
  }

  /**************************************************************************
   * Registration handlers for the asynchronous mode.
   *************************************************************************/

  // -------------------------------------------------------------------------
  // Asynchronous iterator for single row mode.
  // -------------------------------------------------------------------------
  Result &Result::each(std::function<void (Row &)> callback,
                       std::function<void (std::exception_ptr &reason)> error) {

    assert((*handle_->conn).async_);
    assert((*handle_->conn).singleRowMode_);
    
    handle_->each.handle = handle_;
    handle_->each.callback = callback;
    
    if (error) {
      this->error(error);
    }

    return *this;
  }

  // -------------------------------------------------------------------------
  // Asynchronous successful completion.
  // -------------------------------------------------------------------------
  Result &Result::done(std::function<void (Result &)> callback) {

    assert((*handle_->conn).async_);
    handle_->done.handle = handle_;
    handle_->done.callback = callback;

    return *this;
    
  }

  // ---------------------------------------------------------------------------
  // Asynchronous error.
  // ---------------------------------------------------------------------------
  Result &Result::error(std::function<void (std::exception_ptr &)> handler) {

    assert((*handle_->conn).async_);
    if (handle_->lastException) {
      handler(handle_->lastException);
      handle_->clear();
    }
    else {
      handle_->error.handle = handle_;
      handle_->error.callback = handler;
    }
    return *this;
  }
        
        
  // ---------------------------------------------------------------------------
  // Asynchronous completion.
  // ---------------------------------------------------------------------------
  Result &Result::always(std::function<void (Connection &conn) noexcept> handler) {
    
    assert((*handle_->conn).async_);
    if (handle_->lastException) {
      handler(*handle_->conn);
    }
    else {
      handle_->always.handle = handle_;
      handle_->always.callback = handler;
    }
    return *this;
    
  }

} // namespace libpqmxx
