#!/usr/bin/env python3
# -*- coding:utf-8 -*-
import os
import requests
import argparse


root_url = "https://download.amovlab.com/model/SpireCV-models/"
model_list_url = root_url + "model-list.txt"
root_path = os.path.expanduser("~") + "/SpireCV/models"
print("MODEL PATH:", root_path)
if not os.path.exists(root_path):
    os.makedirs(root_path)
list_file = os.path.join(root_path, "model-list.txt")


def main():
    parser = argparse.ArgumentParser(description="SpireCV Model SYNC")
    parser.add_argument(
        "-p", "--platform",
        type=str,
        required=True,
        help="Supported Platforms: nv (Nvidia), int (Intel)",
    )
    args = parser.parse_args()

    if args.platform in ['nv', 'nvidia', 'Nv', 'Nvidia']:
        prefix = 'Nv'
    elif args.platform in ['int', 'intel', 'Int', 'Intel']:
        prefix = 'Int'
    else:
        raise Exception("Platform NOT Support!")

    r = requests.get(model_list_url)
    with open(list_file, "wb") as f:
        f.write(r.content)

    with open(list_file, 'r') as file:
        lines = file.readlines()

    need_switch = False
    for line in lines:
        line = line.strip()
        if len(line) > 0:
            model_file = os.path.join(root_path, line)
            if not os.path.exists(model_file) and (line.startswith(prefix) or line.startswith('Ocv')):
                print("[1] Downloading Model:", line, "...")
                r = requests.get(root_url + line)
                with open(model_file, "wb") as f:
                    f.write(r.content)
                need_switch = True

            if os.path.exists(model_file):
                print("[1] Model:", line, "EXIST!")
                if line.startswith('Nv'):
                    net = line.split('-')[2]
                    if net.startswith("yolov5"):
                        if len(net.split('_')) == 3:
                            name, seg, ncls = net.split('_')
                            engine_fn = os.path.splitext(model_file)[0].replace(net, name + "_" + seg + '_b1_' + ncls) + '.engine'
                            online_fn = os.path.splitext(model_file)[0].replace(net, name + "_" + seg + '_b1_' + ncls) + '-online.engine'
                        else:
                            name, ncls = net.split('_')
                            engine_fn = os.path.splitext(model_file)[0].replace(net, name + '_b1_' + ncls) + '.engine'
                            online_fn = os.path.splitext(model_file)[0].replace(net, name + '_b1_' + ncls) + '-online.engine'
                    else:
                        engine_fn = os.path.splitext(model_file)[0] + '.engine'
                        online_fn = os.path.splitext(model_file)[0] + '-online.engine'
                    if not os.path.exists(engine_fn) and not os.path.exists(online_fn):
                        if net.startswith("yolov5"):
                            if len(net.split('_')) == 3:
                                name, seg, ncls = net.split('_')
                                cmd = "SpireCVSeg -s {} {} {} {}".format(
                                    model_file, engine_fn, ncls[1:], name[6:]
                                )
                            else:
                                name, ncls = net.split('_')
                                cmd = "SpireCVDet -s {} {} {} {}".format(
                                    model_file, engine_fn, ncls[1:], name[6:]
                                )
                        elif line.endswith("onnx"):
                            cmd = ("/usr/src/tensorrt/bin/trtexec --explicitBatch --onnx={} "
                                    "--saveEngine={} --fp16").format(
                                model_file, engine_fn
                            )
                        print("  [2] Converting Model:", line, "->", engine_fn, "...")
                        result = os.popen(cmd).read()
                        need_switch = True
                    else:
                        print("  [2] Model Converting FINISH!")
                    model_file = engine_fn

                if not line.startswith('Ocv') and need_switch:
                    ext = os.path.splitext(model_file)[1]
                    fn_prefix = '-'.join(os.path.basename(model_file).split('-')[:3])
                    file_names = os.listdir(root_path)
                    selected = []
                    for file_name in file_names:
                        if file_name.startswith(fn_prefix) and file_name.endswith(ext):
                            selected.append(file_name)
                    if len(selected) > 0:
                        for i, sel in enumerate(selected):
                            if sel.endswith('-online' + ext):
                                os.rename(
                                    os.path.join(root_path, sel),
                                    os.path.join(root_path, '-'.join(sel.split('-')[:4])) + ext
                                )
                                selected[i] = '-'.join(sel.split('-')[:4]) + ext
                        selected.sort(reverse=True)
                        os.rename(
                            os.path.join(root_path, selected[0]),
                            os.path.join(root_path, os.path.splitext(selected[0])[0] + "-online" + ext)
                        )
                        online_model = os.path.splitext(selected[0])[0] + "-online" + ext
                        print("  [3] Model {} ONLINE *".format(online_model))


if __name__ == "__main__":
    main()
