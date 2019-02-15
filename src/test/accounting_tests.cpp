#include <boost/test/unit_test.hpp>

#include <boost/foreach.hpp>

#include "init.h"
#include "wallet.h"
#include "walletdb.h"

BOOST_AUTO_TEST_SUITE(accounting_tests)

static void
GetResults(CWalletDB& walletdb, std::map<int64, CSTARXuntingEntry>& results)
{
    std::list<CSTARXuntingEntry> aes;

    results.clear();
    BOOST_CHECK(walletdb.ReorderTransactions(pwalletMain) == DB_LOAD_OK);
    walletdb.ListSTARXuntCreditDebit("", aes);
    BOOST_FOREACH(CSTARXuntingEntry& ae, aes)
    {
        results[ae.nOrderPos] = ae;
    }
}

BOOST_AUTO_TEST_CASE(acc_orderupgrade)
{
    CWalletDB walletdb(pwalletMain->strWalletFile);
    std::vector<CWalletTx*> vpwtx;
    CWalletTx wtx;
    CSTARXuntingEntry ae;
    std::map<int64, CSTARXuntingEntry> results;

    ae.strSTARXunt = "";
    ae.nCreditDebit = 1;
    ae.nTime = 1333333333;
    ae.strOtherSTARXunt = "b";
    ae.strComment = "";
    walletdb.WriteSTARXuntingEntry(ae);

    wtx.mapValue["comment"] = "z";
    pwalletMain->AddToWallet(wtx);
    vpwtx.push_back(&pwalletMain->mapWallet[wtx.GetHash()]);
    vpwtx[0]->nTimeReceived = (unsigned int)1333333335;
    vpwtx[0]->nOrderPos = -1;

    ae.nTime = 1333333336;
    ae.strOtherSTARXunt = "c";
    walletdb.WriteSTARXuntingEntry(ae);

    GetResults(walletdb, results);

    BOOST_CHECK(pwalletMain->nOrderPosNext == 3);
    BOOST_CHECK(2 == results.size());
    BOOST_CHECK(results[0].nTime == 1333333333);
    BOOST_CHECK(results[0].strComment.empty());
    BOOST_CHECK(1 == vpwtx[0]->nOrderPos);
    BOOST_CHECK(results[2].nTime == 1333333336);
    BOOST_CHECK(results[2].strOtherSTARXunt == "c");


    ae.nTime = 1333333330;
    ae.strOtherSTARXunt = "d";
    ae.nOrderPos = pwalletMain->IncOrderPosNext();
    walletdb.WriteSTARXuntingEntry(ae);

    GetResults(walletdb, results);

    BOOST_CHECK(results.size() == 3);
    BOOST_CHECK(pwalletMain->nOrderPosNext == 4);
    BOOST_CHECK(results[0].nTime == 1333333333);
    BOOST_CHECK(1 == vpwtx[0]->nOrderPos);
    BOOST_CHECK(results[2].nTime == 1333333336);
    BOOST_CHECK(results[3].nTime == 1333333330);
    BOOST_CHECK(results[3].strComment.empty());


    wtx.mapValue["comment"] = "y";
    --wtx.nLockTime;  // Just to change the hash :)
    pwalletMain->AddToWallet(wtx);
    vpwtx.push_back(&pwalletMain->mapWallet[wtx.GetHash()]);
    vpwtx[1]->nTimeReceived = (unsigned int)1333333336;

    wtx.mapValue["comment"] = "x";
    --wtx.nLockTime;  // Just to change the hash :)
    pwalletMain->AddToWallet(wtx);
    vpwtx.push_back(&pwalletMain->mapWallet[wtx.GetHash()]);
    vpwtx[2]->nTimeReceived = (unsigned int)1333333329;
    vpwtx[2]->nOrderPos = -1;

    GetResults(walletdb, results);

    BOOST_CHECK(results.size() == 3);
    BOOST_CHECK(pwalletMain->nOrderPosNext == 6);
    BOOST_CHECK(0 == vpwtx[2]->nOrderPos);
    BOOST_CHECK(results[1].nTime == 1333333333);
    BOOST_CHECK(2 == vpwtx[0]->nOrderPos);
    BOOST_CHECK(results[3].nTime == 1333333336);
    BOOST_CHECK(results[4].nTime == 1333333330);
    BOOST_CHECK(results[4].strComment.empty());
    BOOST_CHECK(5 == vpwtx[1]->nOrderPos);


    ae.nTime = 1333333334;
    ae.strOtherSTARXunt = "e";
    ae.nOrderPos = -1;
    walletdb.WriteSTARXuntingEntry(ae);

    GetResults(walletdb, results);

    BOOST_CHECK(results.size() == 4);
    BOOST_CHECK(pwalletMain->nOrderPosNext == 7);
    BOOST_CHECK(0 == vpwtx[2]->nOrderPos);
    BOOST_CHECK(results[1].nTime == 1333333333);
    BOOST_CHECK(2 == vpwtx[0]->nOrderPos);
    BOOST_CHECK(results[3].nTime == 1333333336);
    BOOST_CHECK(results[3].strComment.empty());
    BOOST_CHECK(results[4].nTime == 1333333330);
    BOOST_CHECK(results[4].strComment.empty());
    BOOST_CHECK(results[5].nTime == 1333333334);
    BOOST_CHECK(6 == vpwtx[1]->nOrderPos);
}

BOOST_AUTO_TEST_SUITE_END()
