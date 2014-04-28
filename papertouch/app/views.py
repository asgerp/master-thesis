
from flask import render_template, flash, redirect, jsonify, request, url_for
from app import app
from forms import Print4Form, Print8Form, Print12Form, FormSelectCoords, PlacePaperForm
from create_menu import create_menu, make_file
from image_func import crop_image, import_image, write_log_file
import time
import os

@app.route('/')
@app.route('/index')
def index():
    user = { 'nickname': 'Miguel' } # fake user
    return render_template("index.html",
        title = 'Home',
        user = user)
        
@app.route('/train')
def train():
    return render_template("train.html")

@app.route('/success')
def success(folder=None):
    return render_template("success.html",folder=folder)
    
@app.route('/success_trainprint_t')
def success_trainprint_t(folder=None):
    return render_template("success_train.html",folder=folder)

@app.route('/success_print')
def success_print(num=None):
    return render_template("success_print.html",num=num)


@app.route('/success_trainprint')
def success_trainprint(num=None):
    return render_template("success_trainprint.html",num=num)


@app.errorhandler(404)
def internal_error(error):
    return render_template('404.html'), 404

@app.errorhandler(500)
def internal_error(error):
    db.session.rollback()
    return render_template('500.html'), 500


@app.route('/train_print_4')
def train_trial4():
    cb = str(time.time())
    return render_template("print4train_trials.html",cb=cb)    
    
@app.route('/trial_print_4')
def trial4():
    cb = str(time.time())
    return render_template("print4_trials.html",cb=cb)

@app.route('/trial_print_8')
def trial8():
    cb = str(time.time())
    return render_template("print8_trials.html",cb=cb)

@app.route('/trial_print_12')
def trial12():
    cb = str(time.time())
    return render_template("print12_trials.html",cb=cb)
    
@app.route('/trial_tegn4')
def trial4t():
    cb = str(time.time())
    return render_template("tegn4_trials.html",cb=cb)

@app.route('/trial_tegn8')
def trial8t():
    cb = str(time.time())
    return render_template("tegn8_trials.html",cb=cb)

@app.route('/trial_tegn12')
def trial12t():
    cb = str(time.time())
    return render_template("tegn12_trials.html",cb=cb)


@app.route('/start_prog')
def start_prog():
    folder = request.args.get('folder', '')
    script = ""
    if folder == '':
        script = 'print.command'
    else:
        script = folder + '.command'
    os.system('sh /Users/mundane/Desktop/' + script + '&')
    return jsonify()

@app.route('/place_paper_train', methods=['GET', 'POST'])
def place_image_train():
    form = PlacePaperForm()
    num = request.args.get('num', 0, type=int)
    if form.validate_on_submit():
        if form.number.data == u'4':
            return tegn4train(form.image.data)
        if form.number.data == u'8':
            return tegn8(form.image.data)
        if form.number.data == u'12':
            return tegn12(form.image.data)
    return render_template('place_paper_train.html', num=num, form=form)

@app.route('/place_paper', methods=['GET', 'POST'])
def place_image():
    form = PlacePaperForm()
    num = request.args.get('num', 0, type=int)
    if form.validate_on_submit():
        if form.number.data == u'4':
            return tegn4(form.image.data)
        if form.number.data == u'8':
            return tegn8(form.image.data)
        if form.number.data == u'12':
            return tegn12(form.image.data)
    return render_template('place_paper.html', num=num, form=form)

@app.route('/write_logs')
def write_log():
    times = request.args.getlist('times')
    conf = request.args.get('conf','')
    write_log_file(times,conf)
    os.system('sh /Users/mundane/Desktop/kill.command &')
    return jsonify(result=True)


@app.route('/get_img')
def get_img():
    filename = import_image();
    return jsonify(result=filename)

@app.route('/get_img_icon')
def get_img_icon():
    x1       = request.args.get('x1', 0, type=int)
    y1       = request.args.get('y1', 0, type=int)
    x2       = request.args.get('x2', 0, type=int)
    y2       = request.args.get('y2', 0, type=int)
    filename = request.args.get('path', '')
    marker   = request.args.get('marker', '')
    config   = request.args.get('config', '')
    crop_file = crop_image(x1,y1,x2,y2, filename, marker, config)
    return jsonify(result=crop_file+'?cb='+str(time.time()),marker=marker)
    

@app.route('/tegn4train', methods=['GET', 'POST'])
def tegn4train(img=None):
    image = str(img)
    form = FormSelectCoords()
    if form.validate_on_submit():
        return success_trainprint_t('tegn4')
    return render_template('tegn4train.html', form=form, image=image)


@app.route('/tegn4', methods=['GET', 'POST'])
def tegn4(img=None):
    image = str(img)
    form = FormSelectCoords()
    if form.validate_on_submit():
        return success('tegn4')
        #return redirect(url_for('success', folder='tegn4'))
    return render_template('tegn4.html', form=form, image=image)
    
@app.route('/tegn8', methods=['GET', 'POST'])
def tegn8(img=None):
    image = str(img)
    form = FormSelectCoords()
    if form.validate_on_submit():
        return success('tegn8')
    return render_template('tegn8.html', form=form, image=image)
    
@app.route('/tegn12', methods=['GET', 'POST'])
def tegn12(img=None):
    image = str(img)
    form = FormSelectCoords()
    if form.validate_on_submit():
        return success('tegn12')
    return render_template('tegn12.html', form=form, image=image)

@app.route('/trainprint4', methods=['GET', 'POST'])
def trainprint4():
    form = Print4Form()
    if form.validate_on_submit():
        create_menu(4, [form.up_left.data,
                        form.up_right.data,
                        form.down_left.data,
                        form.down_right.data])
        make_file("menu4")
        return success_trainprint('4')
    return render_template('trainprint4.html', form=form)


@app.route('/print4', methods=['GET', 'POST'])
def print4():
    form = Print4Form()
    if form.validate_on_submit():
        create_menu(4, [form.up_left.data,
                        form.up_right.data,
                        form.down_left.data,
                        form.down_right.data])
        make_file("menu4")
        return success_print('4')
    return render_template('print4.html', form=form)

@app.route('/print8', methods=['GET', 'POST'])
def print8():
    form = Print8Form()
    if form.validate_on_submit():
        create_menu(8, [form.up_left.data,
                        form.up_mid.data,
                        form.up_right.data,
                        form.mid_left.data,
                        form.mid_right.data,
                        form.down_left.data,
                        form.down_mid.data,
                        form.down_right.data])
        make_file("menu8")
        return success_print('8')
    return render_template('print8.html', form=form)

@app.route('/print12', methods=['GET', 'POST'])
def print12():
    form = Print12Form()
    if form.validate_on_submit():
        create_menu(12, [form.up_left.data,
                        form.up_mid.data,
                        form.up_right.data,
                        form.mid_up_left.data,
                        form.mid_up_mid.data,
                        form.mid_up_right.data,
                        form.mid_left.data,
                        form.mid_mid.data,
                        form.mid_right.data,
                        form.down_left.data,
                        form.down_mid.data,
                        form.down_right.data])
        make_file("menu12")
        return success_print('12')
    return render_template('print12.html', form=form)

