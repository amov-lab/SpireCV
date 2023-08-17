from pycocotools.coco import COCO
from pycocotools.cocoeval import COCOeval
import os
import json

if __name__ == '__main__':
    path = os.path.abspath(os.path.join(os.getcwd(),"../../.."))
    pred_json = 'pd_coco.json'
    anno_json = path + '/val2017/instances_val2017.json'
    
    # use COCO API to load forecast results and annotations
    cocoGt = COCO(anno_json)
    with open(pred_json,'r') as file:
        data = json.load(file)
    
    # align anno_json with pred_json category_id
    gtCatDicts = {}
    for anns in range(len(cocoGt.getCatIds())):
        gtCatDicts[anns] = cocoGt.getCatIds()[anns]

    pdCatIds=list(set([d['category_id'] for d in data]))

    if not set(pdCatIds).issubset(set(cocoGt.getCatIds())):
        for ins in data:
            temp = int(gtCatDicts[ins['category_id']])
            ins['category_id'] = temp

    # load prediction results
    cocoDt = cocoGt.loadRes(data)
 
    # create COCO eval object
    cocoEval = COCOeval(cocoGt, cocoDt,'bbox')

    # assessment
    cocoEval.evaluate()
    cocoEval.accumulate()
    cocoEval.summarize()
