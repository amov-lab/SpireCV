python3 run_mot_challenge.py \
--BENCHMARK MOT17 \
--GT_FOLDER ../../../dataset/MOT17/train \
--TRACKERS_FOLDER ../../../dataset/pred_mot17 \
--SKIP_SPLIT_FOL True \
--SEQMAP_FILE ../../../dataset/MOT17/val_seqmap.txt \
--TRACKERS_TO_EVAL '' \
--METRICS HOTA CLEAR Identity \
#--show
