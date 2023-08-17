import datetime
import json
import os
import cv2

# revert prediction results to coco_json 
path = os.path.abspath(os.path.join(os.getcwd(), "../../.."))

# all files dir
images_path = path + '/val2017/val2017'
preds_path = path + '/val2017/preds'
coco_json_save ='pd_coco.json'

# config coco_json
coco_json = []

# load images dir
images = os.listdir(images_path)
for image in images:
    # get image name
    image_name, image_suffix = os.path.splitext(image)
    # get image W and H
    image_path = images_path + '/' + image
    img = cv2.imread(image_path)
    height, width, _ = img.shape

    # read pred's txt
    pred_path = preds_path + '/' + image_name + '.txt'
    if not os.path.exists(pred_path):
        continue
    with open(pred_path, 'r') as f:
        preds = f.readlines()
        preds = [l.strip() for l in preds]
        for j,pred in enumerate(preds):
            pred = pred.split(' ')
            category_id = int(pred[0])
            x = float(pred[1]) * width
            y = float(pred[2]) * height
            w = float(pred[3]) * width
            h = float(pred[4]) * height
            xmin = x - w / 2
            ymin = y - h / 2
            
            coco_json.append({
                'image_id': int(image_name),
                'category_id': int(category_id),
                'bbox': [xmin, ymin, w, h],
                'score': float(pred[5])
            })

# save json
with open(os.path.join(coco_json_save), 'w') as f:
    json.dump(coco_json, f, indent=2)
print(len(coco_json), 'Done!')
