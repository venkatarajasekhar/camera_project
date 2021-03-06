#ifndef _ZDSHARED_H_
#define _ZDSHARED_H_

void mkFragment(Signal_t *signal, FrmDesc_t *pfrmDesc);
BOOLEAN sendMgtFrame(Signal_t *signal, FrmDesc_t *pfrmDesc);
BOOLEAN	getElem(Frame_t *frame, ElementID eID, Element *eElem);

void mkAuthFrm(FrmDesc_t* pfrmDesc, MacAddr_t *addr1, U16 Alg, U16 Seq, 
	U16 Status, U8 *pChalng, U8 vapId);
void mkRe_AsocRspFrm(FrmDesc_t* pfrmDesc, TypeSubtype subType, MacAddr_t *addr1, 
	U16 Cap, U16 Status, U16 Aid, Element *pSupRates, Element *pExtRates, U8 vapId);
void mkProbeRspFrm(FrmDesc_t* pfrmDesc, MacAddr_t *addr1, U16 BcnInterval, 
	U16 Cap, Element *pSsid, Element *pSupRates, Element *pDsParms, 
	Element *pExtRates, Element *pWpa, U8 vapId);
void mkDisAssoc_DeAuthFrm(FrmDesc_t* pfrmDesc, TypeSubtype subType, MacAddr_t *addr1, 
	U16 Reason, U8 vapId);		
void sendProbeRspFrm(MacAddr_t *addr1, U16 BcnInterval, U16 Cap, 
	Element *pSsid, Element *pSupRates, Element *pDsParms, 
	Element *pExtRates, Element *pWpa, U8 vapId);
		
#endif
