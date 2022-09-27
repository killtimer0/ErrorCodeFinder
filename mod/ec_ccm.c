#include "../ecf.h"

#define CCM_E_ITEMNOTFOUND                      0x87D00215
#define CCM_E_EMPTY_CERT_STORE                  0x87D00280
#define CCM_E_NO_CERT_MATCHING_CRITERIA         0x87D00281
#define CCM_E_TOO_MANY_CERTS                    0x87D00282
#define CCM_E_MISSING_PRIVATEKEY                0x87D00283
#define CCM_E_MISSING_SUBJECT_NAME              0x87D00284
#define CCM_E_UNKNOWN_SEARCH_CRITERIA           0x87D00285
#define CCM_E_INVALID_SMS_AUTHORITY             0x87D00286
#define CCM_E_MISSING_SITE_SIGNING_CERT         0x87D00287
#define CCM_E_CI_DECOMPRESSION_FAILURE          0x87D00288
#define CCM_E_CI_DEFN_DECOMPRESSION_FAILURE     0x87D00289
#define CCM_E_JOB_EMPTY                         0x87D00290
#define CCM_E_TASKSEQUENCE_NOT_ASSIGNED         0x87D00291
#define CCM_E_COMPUTE_SIGNATURE                 0x87D00292
#define CCM_E_REFRESH_SSSC                      0x87D00293
#define CCM_E_VERIFY_POLICY                     0x87D00294
#define CCM_E_INVALID_OR_NO_REGISTRATION_CERT   0x87D00295
#define CCM_E_CI_PROCESSING_FAILED              0x87D00296
#define CCM_E_INVALID_KEY                       0x87D00297
#define CCM_E_INVALID_REGISTRATION              0x87D00298
#define CCM_E_INVALID_SIGNATURE_TYPE            0x87D00299
#define CCM_E_INVALID_AUTH_HEADER               0x87D0029A
#define CCM_E_AUTHENTICATOR_EXPIRED             0x87D0029B
#define CCM_E_RESET_REGISTRATION                0x87D0029C
#define CCM_E_SITE_INCOMPATIBLE                 0x87D0029D
#define CCM_E_HASH_MISMATCH                     0x87D0029E
#define CCM_E_CERTENROLL_SCEP_CERTREQUEST_PENDI 0x87D00900
#define CCM_E_CERTENROLL_SCEP_CERTREQUEST_UNEXP 0x87D00901
#define CCM_E_CERTENROLL_SCEP_CERTREQUEST_FAILU 0x87D00902
#define CCM_E_CERTENROLL_SCEP_CERTREQUEST_BADAL 0x87D00903
#define CCM_E_CERTENROLL_SCEP_CERTREQUEST_BADME 0x87D00904
#define CCM_E_CERTENROLL_SCEP_CERTREQUEST_BADTR 0x87D00905
#define CCM_E_CERTENROLL_SCEP_CERTREQUEST_BADSI 0x87D00906
#define CCM_E_CERTENROLL_SCEP_CERTREQUEST_BADCE 0x87D00907
#define CCM_E_CERTENROLL_SCEP_SERVERCERT_EMPTY  0x87D00908
#define CCM_E_CERTENROLL_SCEP_SERVERCAP_EMPTY   0x87D00909
#define CCM_E_CERTENROLL_SCEP_PKIOPRESPONSE_EMP 0x87D00910
#define CCM_E_CERTENROLL_SCEP_TPM_UNAVAILABLE   0x87D00911

static ErrorCodeItem ec_ccm_items[] = {
    REGCODE(CCM_E_ITEMNOTFOUND)
    REGCODE(CCM_E_EMPTY_CERT_STORE)
    REGCODE(CCM_E_NO_CERT_MATCHING_CRITERIA)
    REGCODE(CCM_E_TOO_MANY_CERTS)
    REGCODE(CCM_E_MISSING_PRIVATEKEY)
    REGCODE(CCM_E_MISSING_SUBJECT_NAME)
    REGCODE(CCM_E_UNKNOWN_SEARCH_CRITERIA)
    REGCODE(CCM_E_INVALID_SMS_AUTHORITY)
    REGCODE(CCM_E_MISSING_SITE_SIGNING_CERT)
    REGCODE(CCM_E_CI_DECOMPRESSION_FAILURE)
    REGCODE(CCM_E_CI_DEFN_DECOMPRESSION_FAILURE)
    REGCODE(CCM_E_JOB_EMPTY)
    REGCODE(CCM_E_TASKSEQUENCE_NOT_ASSIGNED)
    REGCODE(CCM_E_COMPUTE_SIGNATURE)
    REGCODE(CCM_E_REFRESH_SSSC)
    REGCODE(CCM_E_VERIFY_POLICY)
    REGCODE(CCM_E_INVALID_OR_NO_REGISTRATION_CERT)
    REGCODE(CCM_E_CI_PROCESSING_FAILED)
    REGCODE(CCM_E_INVALID_KEY)
    REGCODE(CCM_E_INVALID_REGISTRATION)
    REGCODE(CCM_E_INVALID_SIGNATURE_TYPE)
    REGCODE(CCM_E_INVALID_AUTH_HEADER)
    REGCODE(CCM_E_AUTHENTICATOR_EXPIRED)
    REGCODE(CCM_E_RESET_REGISTRATION)
    REGCODE(CCM_E_SITE_INCOMPATIBLE)
    REGCODE(CCM_E_HASH_MISMATCH)
    REGCODE(CCM_E_CERTENROLL_SCEP_CERTREQUEST_PENDI)
    REGCODE(CCM_E_CERTENROLL_SCEP_CERTREQUEST_UNEXP)
    REGCODE(CCM_E_CERTENROLL_SCEP_CERTREQUEST_FAILU)
    REGCODE(CCM_E_CERTENROLL_SCEP_CERTREQUEST_BADAL)
    REGCODE(CCM_E_CERTENROLL_SCEP_CERTREQUEST_BADME)
    REGCODE(CCM_E_CERTENROLL_SCEP_CERTREQUEST_BADTR)
    REGCODE(CCM_E_CERTENROLL_SCEP_CERTREQUEST_BADSI)
    REGCODE(CCM_E_CERTENROLL_SCEP_CERTREQUEST_BADCE)
    REGCODE(CCM_E_CERTENROLL_SCEP_SERVERCERT_EMPTY)
    REGCODE(CCM_E_CERTENROLL_SCEP_SERVERCAP_EMPTY)
    REGCODE(CCM_E_CERTENROLL_SCEP_PKIOPRESPONSE_EMP)
    REGCODE(CCM_E_CERTENROLL_SCEP_TPM_UNAVAILABLE)
};

static LPTSTR ccm_code2msg(DWORD dwCode)
{
    LPTSTR result = NULL;

    if (hModCertUtil)
    {
        UINT uID = dwCode - 0x87D00000 + 0xE000;

        if (0 != uID)
        {
            result = (LPTSTR)LocalAlloc(LPTR, MAX_MSG_LEN * sizeof (TCHAR));
            if (result)
            {
                LoadString(hModCertUtil, uID, result, MAX_MSG_LEN);
            }
        }
    }

    return result;
}

static void ccm_freemsg(LPTSTR lpMessage)
{
    LocalFree(lpMessage);
}

void EcRegCcm(ErrorCodeSet *pecs)
{
    pecs->lpName = TEXT("CCM");
    pecs->pfnMessageFromCode = ccm_code2msg;
    pecs->pfnFreeMessage = ccm_freemsg;
    pecs->items = ec_ccm_items;
    pecs->dwCodeCount = ARRAYSIZE(ec_ccm_items);
}

void EcUnregCcm()
{
    /* Ignore */
}

