#include "XQFunction.h"

namespace sedna
{
    // several internal mergers to receive result specs from param specs

    // result determined entirely by the first param spec
    // example: fn:reverse, fn:subsequence, etc.
    xqExprInfo firstArgResult(const std::vector<xqExprInfo> &params)
    {
        return params[0];
    }

    // result is an atomic (e.g. boolean) or one-node
    xqExprInfo resultOne(const std::vector<xqExprInfo> &params)
    {
        xqExprInfo res = {true, true, true, true, false};
        return res;
    }

    // result is an atomic sequence (e.g. xs:integer*)
    // example: fn:data
    xqExprInfo atomicResultMany(const std::vector<xqExprInfo> &params)
    {
        xqExprInfo res = {true, true, false, true, false};
        return res;
    }

    // function may result in any sequence
    xqExprInfo resultUndefined(const std::vector<xqExprInfo> &params)
    {
        xqExprInfo res = {false, false, false, false, true};
        return res;
    }

    // function may result in any sequence, but without tmp-nodes
    xqExprInfo resultUndefinedWoConst(const std::vector<xqExprInfo> &params)
    {
        xqExprInfo res = {false, false, false, false, false};
        return res;
    }

    // standard and additional functions
    XQFunction xqueryFunctions[] =
    {
        {"", "abs",  1,  1, 0xFF, "!fn!abs", false, &resultOne},
        {"", "adjust-dateTime-to-timezone",  1,  2, 0xFF, "!fn!adjust-dateTime-to-timezone", false, &resultOne},
        {"", "adjust-date-to-timezone",  1,  2, 0xFF, "!fn!adjust-date-to-timezone", false, &resultOne},
        {"", "adjust-time-to-timezone",  1,  2, 0xFF, "!fn!adjust-time-to-timezone", false, &resultOne},
        {"", "avg",  1,  1, 0xFF, "!fn!avg", false, &resultOne},
        {"", "base-uri",  0,  1, 0xFF, "!fn!base-uri", false, &resultOne},
        {"", "boolean",  1,  1, 0xFF, "!fn!boolean", false, &resultOne},
        {"", "ceiling",  1,  1, 0xFF, "!fn!ceiling", false, &resultOne},
        {"", "codepoint-equal",  2,  2, 0xFF, "!fn!codepoint-equal", false, &resultOne},
        {"", "codepoints-to-string",  1,  1, 0xFF, "!fn!codepoints-to-string", false, &resultOne},
        {"", "collection",  1,  1, 0xFF, "!fn!collection", false, &atomicResultMany}, // assume rsult is ddoed, so I use this merger
        {"", "compare",  2,  3, 0xFF, "!fn!compare", false, &resultOne},
        {"", "concat",  2,  UINT32_MAX, 0xFF, "!fn!concat", false, &resultOne},
        {"", "contains",  2,  3, 0xFF, "!fn!contains", false, &resultOne},
        {"", "count",  1,  1, 0xFF, "!fn!count", false, &resultOne},
        {"", "current-date",  0,  0, 0xFF, "!fn!current-date", false, &resultOne},
        {"", "current-dateTime",  0,  0, 0xFF, "!fn!current-dateTime", false, &resultOne},
        {"", "current-time",  0,  0, 0xFF, "!fn!current-time", false, &resultOne},
        {"", "data",  1, 1, 0, "!fn!data", true, &atomicResultMany},
        {"", "dateTime",  2,  2, 0xFF, "!fn!dateTime", false, &resultOne},
        {"", "day-from-date",  1,  1, 0xFF, "!fn!day-from-date", false, &resultOne},
        {"", "day-from-dateTime",  1,  1, 0xFF, "!fn!day-from-dateTime", false, &resultOne},
        {"", "days-from-duration",  1,  1, 0xFF, "!fn!days-from-duration", false, &resultOne},
        {"", "deep-equal",  2,  3, 0xFC, "!fn!deep-equal", true, &resultOne},
        {"", "default-collation",  0,  0, 0xFF, "!fn!default-collation", false, &resultOne},
        {"", "distinct-values",  1,  2, 0xFE, "!fn!distinct-values", false, &atomicResultMany},
        {"", "doc",  1,  2, 0xFF, "!fn!document", false, &resultOne},
        {"", "doc-available",  1,  1, 0xFF, "!fn!doc-available", false, &resultOne},
        {"", "document",  1,  2, 0xFF, "!fn!document", false, &resultOne},
        {"", "document-uri",  1,  1, 0xFF, "!fn!document-uri", false, &resultOne},
        {"", "empty",  1,  1, 0xFF, "!fn!empty", false, &resultOne},
        {"", "encode-for-uri",  1,  1, 0xFF, "!fn!encode-for-uri", false, &resultOne},
        {"", "ends-with",  2,  3, 0xFF, "!fn!ends-with", false, &resultOne},
        {"", "error",  0,  3, 0xFF, "!fn!error", false, &resultOne},
        {"", "escape-html-uri",  1,  1, 0xFF, "!fn!escape-html-uri", false, &resultOne},
        {"", "exactly-one",  1,  1, 0xFF, "!fn!exactly-one", false, &resultOne},
        {"", "exists",  1,  1, 0xFF, "!fn!exists", false, &resultOne},
        {"", "false",  0,  0, 0xFF, "!fn!false", false, &resultOne},
        {"", "filter_entry_level",  1,  1, 0, "!fn!filter_entry_level", false, &firstArgResult},
        {"", "floor",  1,  1, 0xFF, "!fn!floor", false, &resultOne},
        {"", "fthighlight",  2,  3, 0xFF, "!fn!fthighlight", false, &atomicResultMany},
        {"", "fthighlight-blocks",  2,  3, 0xFF, "!fn!fthighlight-blocks", false, &atomicResultMany},
        {"", "ftindex-scan",  2,  3, 0xFF, "!fn!ftindex-scan", false, &resultUndefinedWoConst},
        {"", "ftscan",  3,  4, 0xFE, "!fn!ftscan", false, &firstArgResult},
        {"", "ftwindex-scan",  2,  4, 0xFF, "!fn!windex-scan", false, &resultUndefinedWoConst},
        {"", "hours-from-dateTime",  1,  1, 0xFF, "!fn!hours-from-dateTime", false, &resultOne},
        {"", "hours-from-duration",  1,  1, 0xFF, "!fn!hours-from-duration", false, &resultOne},
        {"", "hours-from-time",  1,  1, 0xFF, "!fn!hours-from-time", false, &resultOne},
        {"", "id",  1,  2, 0xFF, "!fn!id", true, &resultUndefined},
        {"", "idref",  1,  2, 0xFF, "!fn!idref", true, &resultUndefined},
        {"", "implicit-timezone",  0,  0, 0xFF, "!fn!implicit-timezone", false, &resultOne},
        {"", "index-of",  2,  3, 0xFE, "!fn!index-of", false, &atomicResultMany},
        {"", "index-scan",  3,  3, 0xFF, "!fn!index-scan", false, &resultUndefinedWoConst},
        {"", "index-scan-between",  4,  4, 0xFF, "!fn!index-scan-between", false, &resultUndefinedWoConst},
        {"", "in-scope-prefixes",  1,  1, 0xFF, "!fn!in-scope-prefixes", false, &atomicResultMany},
        {"", "insert-before",  3,  3, 0xFA, "!fn!insert-before", false, &resultUndefined},
        {"", "iri-to-uri",  1,  1, 0xFF, "!fn!iri-to-uri", false, &resultOne},
        {"", "is_ancestor",  2,  2, 0, "!fn!is_ancestor", false, &resultOne},
        {"", "item-at",  2,  2, 0xFE, "!fn!item-at", false, &resultOne},
        {"", "lang",  1,  2, 0xFF, "!fn!lang", true, &resultUndefined},
        {"", "last",  0,  0, 0xFF, "!fn!last", false, &resultOne},
        {"", "local-name",  0,  1, 0xFF, "!fn!local-name", false, &resultOne},
        {"", "local-name-from-QName",  1,  1, 0xFF, "!fn!local-name-from-QName", false, &resultOne},
        {"", "lower-case",  1,  1, 0xFF, "!fn!lower-case", false, &resultOne},
        {"", "matches",  2,  3, 0xFF, "!fn!matches", false, &resultOne},
        {"", "max",  1,  2, 0xFF, "!fn!max", false, &resultOne},
        {"", "min",  1,  2, 0xFF, "!fn!min", false, &resultOne},
        {"", "minutes-from-dateTime",  1,  1, 0xFF, "!fn!minutes-from-dateTime", false, &resultOne},
        {"", "minutes-from-duration",  1,  1, 0xFF, "!fn!minutes-from-duration", false, &resultOne},
        {"", "minutes-from-time",  1,  1, 0xFF, "!fn!minutes-from-time", false, &resultOne},
        {"", "month-from-date",  1,  1, 0xFF, "!fn!month-from-date", false, &resultOne},
        {"", "month-from-dateTime",  1,  1, 0xFF, "!fn!month-from-dateTime", false, &resultOne},
        {"", "months-from-duration",  1,  1, 0xFF, "!fn!months-from-duration", false, &resultOne},
        {"", "name",  0,  1, 0xFF, "!fn!name", false, &resultOne},
        {"", "namespace-uri",  0,  1, 0xFF, "!fn!namespace-uri", false, &resultOne},
        {"", "namespace-uri-for-prefix",  2,  2, 0xFF, "!fn!namespace-uri-for-prefix", false, &resultOne},
        {"", "namespace-uri-from-QName",  1,  1, 0xFF, "!fn!namespace-uri-from-QName", false, &resultOne},
        {"", "nilled", 1, 1, 0xFF, "!fn!nilled", false, &resultOne},
        {"", "node-kind",  1,  1, 0xFF, "!fn!node-kind", false, &resultOne},
        {"", "node-name", 1, 1, 0xFF, "!fn!node-name", false, &resultOne},
        {"", "normalize-space",  0,  1, 0xFF, "!fn!normalize-space", false, &resultOne},
        {"", "normalize-unicode",  1,  2, 0xFF, "!fn!normalize-unicode", false, &resultOne},
        {"", "not",  1,  1, 0xFF, "!fn!not", false, &resultOne},
        {"", "number",  0,  1, 0xFF, "!fn!number", false, &resultOne},
        {"", "one-or-more",  1,  1, 0xFF, "!fn!one-or-more", false, &firstArgResult},
        {"", "position",  0,  0, 0xFF, "!fn!position", false, &resultOne},
        {"", "prefix-from-QName",  1,  1, 0xFF, "!fn!prefix-from-QName", false, &resultOne},
        {"", "QName",  2,  2, 0xFF, "!fn!QName", false, &resultOne},
        {"", "remove",  2,  2, 0xFE, "!fn!remove", false, &firstArgResult},
        {"", "replace",  3,  4, 0xFF, "!fn!replace", false, &resultOne},
        {"", "resolve-QName",  2,  2, 0xFF, "!fn!resolve-QName", false, &resultOne},
        {"", "resolve-uri",  1,  2, 0xFF, "!fn!resolve-uri", false, &resultOne},
        {"", "reverse",  1,  1, 0, "!fn!reverse", false, &firstArgResult},
        {"", "root",  0,  1, 0xFF, "!fn!root", false, &resultOne},
        {"", "round",  1,  1, 0xFF, "!fn!round", false, &resultOne},
        {"", "round-half-to-even",  1,  2, 0xFF, "!fn!round-half-to-even", false, &resultOne},
        {"", "seconds-from-dateTime",  1,  1, 0xFF, "!fn!seconds-from-dateTime", false, &resultOne},
        {"", "seconds-from-duration",  1,  1, 0xFF, "!fn!seconds-from-duration", false, &resultOne},
        {"", "seconds-from-time",  1,  1, 0xFF, "!fn!seconds-from-time", false, &resultOne},
        {"", "starts-with",  2,  3, 0xFF, "!fn!starts-with", false, &resultOne},
        {"", "static-base-uri",  0,  0, 0xFF, "!fn!static-base-uri", false, &resultOne},
        {"", "string",  0,  1, 0xFF, "!fn!string", false, &resultOne},
        {"", "string-join",  2,  2, 0xFF, "!fn!string-join", false, &resultOne},
        {"", "string-length",  0,  1, 0xFF, "!fn!string-length", false, &resultOne},
        {"", "string-to-codepoints",  1,  1, 0xFF, "!fn!string-to-codepoints", false, &resultOne},
        {"", "string-value",  1,  1, 0xFF, "!fn!string-value", false, &resultOne},
        {"", "subsequence",  2,  3, 0xFE, "!fn!subsequence", false, &firstArgResult},
        {"", "substring",  2,  3, 0xFF, "!fn!substring", false, &resultOne},
        {"", "substring-after",  2,  3, 0xFF, "!fn!substring-after", false, &resultOne},
        {"", "substring-before",  2,  3, 0xFF, "!fn!substring-before", false, &resultOne},
        {"", "sum",  1,  2, 0xFF, "!fn!sum", false, &resultOne},
        {"", "test",  0,  UINT32_MAX, 0xFF, "!fn!test", false, &resultOne},
        {"", "timezone-from-date",  1,  1, 0xFF, "!fn!timezone-from-date", false, &resultOne},
        {"", "timezone-from-dateTime",  1,  1, 0xFF, "!fn!timezone-from-dateTime", false, &resultOne},
        {"", "timezone-from-time",  1,  1, 0xFF, "!fn!timezone-from-time", false, &resultOne},
        {"", "tokenize",  2,  3, 0xFF, "!fn!tokenize", false, &atomicResultMany},
        {"", "trace",  2,  2, 0xFF, "!fn!trace", false, &firstArgResult},
        {"", "translate",  3,  3, 0xFF, "!fn!translate", false, &resultOne},
        {"", "true",  0,  0, 0xFF, "!fn!true", false, &resultOne},
        {"", "typed-value",  1,  1, 0xFF, "!fn!typed-value", false, &resultOne},
        {"", "unordered",  1,  1, 0, "!fn!unordered", false, &firstArgResult},
        {"", "upper-case",  1,  1, 0xFF, "!fn!upper-case", false, &resultOne},
        {"", "year-from-date",  1,  1, 0xFF, "!fn!year-from-date", false, &resultOne},
        {"", "year-from-dateTime",  1,  1, 0xFF, "!fn!year-from-dateTime", false, &resultOne},
        {"", "years-from-duration",  1,  1, 0xFF, "!fn!years-from-duration", false, &resultOne},
        {"", "zero-or-one",  1,  1, 0, "!fn!zero-or-one", false, &firstArgResult},
        {"", "", 0, 0, 0xFF, "", false, &resultOne} // dummy record to mark the end
    };

    XQFunction sqlFunctions[] =
    {
        {"", "close",  1,  1, 0xFF, "!fn!sql-close", false, &resultOne},
        {"", "commit",  1,  1, 0xFF, "!fn!sql-commit", false, &resultOne},
        {"", "connect",  1,  4, 0xFF, "!fn!sql-connect", false, &resultOne},
        {"", "exec-update",  2,  UINT32_MAX, 0xFF, "!fn!sql-exec-update", false, &resultOne},
        {"", "execute",  2,  UINT32_MAX, 0xFF, "!fn!sql-execute", false, &resultUndefinedWoConst},
        {"", "prepare",  2,  3, 0xFF, "!fn!sql-prepare", false, &resultOne},
        {"", "rollback",  1,  1, 0xFF, "!fn!sql-rollback", false, &resultOne},
        {"", "", 0, 0, 0xFF, "", false, &resultOne} // dummy record to mark the end
    };

    XQFunction seFunctions[] =
    {
        {"", "checkpoint",  0,  0, 0xFF, "!se!checkpoint", false, &resultOne},
        {"", "get-property",  1,  1, 0xFF, "!se!get-property", false, &resultOne},
        {"", "", 0, 0, 0xFF, "", false, &resultOne} // dummy record to mark the end
    };
}