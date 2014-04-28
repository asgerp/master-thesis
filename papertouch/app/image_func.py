from __future__ import print_function
from PIL import Image
import time
import csv 
import json
from pprint import pprint


import os
from app import app


global ext, pre
ext = ".png"
pre = os.path.dirname(os.path.realpath(__file__)) +"/static/img/"

def crop_image(x1,y1,x2,y2, file, marker, config):
    

    file = str(file)
    file = os.path.dirname(os.path.realpath(__file__)) + file
    im = Image.open(file)
    cropped_file = config + "/" + marker + "_cropped" + ext
    box = (x1, y1, x2, y2)
    region = im.crop(box)
    hsize = 0
    basewidth = 220
    while hsize < 200:
        basewidth += 1
        wpercent = (basewidth/float(region.size[0]))
    
        hsize = int((float(region.size[1])*float(wpercent)))
        
    region = region.resize((basewidth,hsize), Image.ANTIALIAS)

    if os.path.isfile(pre + cropped_file):
        os.remove(pre + cropped_file)
    region.save(pre + cropped_file)
    return "/static/img/" + cropped_file
    
def import_image():
    f_name = 'import_menu_'+str(time.time())+ext
    pwd = os.path.dirname(os.path.realpath(__file__))
    os.system(pwd + '/image_grabber ' + pre + f_name)
    return "/static/img/" + f_name


def write_log_file(data,conf):
    file = os.path.dirname(os.path.realpath(__file__)) + '/LOGFILE_'+str(conf)+'_'+str(time.time()) + '.json'
    f = open(file, 'w')
    for item in data:
      f.write("%s\n" % item.encode('ascii','ignore'))
    f.close()
    write_csv_file(file)

def write_csv_file(f_name):
    json_data=open(f_name)

    data = json.load(json_data)

    file = f_name + '.csv'
    fp = open(file, "wb+")
    f = csv.writer(fp)
    #file = os.path.dirname(os.path.realpath(__file__)) + '/LOGFILE_'+str(conf)+'_'+str(time.time())
    #f = open(file, 'w')
    f.writerow(["Icon", "starttime"])
    for x in data:
        f.writerow([x['icon'],
                   x['start']])
    fp.close()
    