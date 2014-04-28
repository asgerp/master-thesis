from flask_wtf import Form
from wtforms import validators, HiddenField, FieldList, FormField
from wtforms.validators import InputRequired

class Print4Form(Form):
    up_left = HiddenField('up_left', validators=[InputRequired()])
    up_right = HiddenField('up_right',validators=[InputRequired()])
    down_left = HiddenField('down_left',validators=[InputRequired()])
    down_right = HiddenField('down_right',validators=[InputRequired()])

class Print8Form(Form):
    up_left = HiddenField('up_left', validators=[InputRequired()])
    up_mid = HiddenField('up_mid', validators=[InputRequired()])
    up_right = HiddenField('up_right',validators=[InputRequired()])
    mid_left = HiddenField('mid_left',validators=[InputRequired()])
    mid_right = HiddenField('mid_right',validators=[InputRequired()])
    down_left = HiddenField('down_left',validators=[InputRequired()])
    down_mid = HiddenField('down_mid',validators=[InputRequired()])
    down_right = HiddenField('down_right',validators=[InputRequired()])

class PlacePaperForm(Form):
    image = HiddenField('image', validators=[InputRequired()])
    number = HiddenField('number', validators=[InputRequired()])

class Print12Form(Form):
    up_left = HiddenField('up_left', validators=[InputRequired()])
    up_mid = HiddenField('up_mid', validators=[InputRequired()])
    up_right = HiddenField('up_right',validators=[InputRequired()])
    mid_up_left = HiddenField('mid_up_left',validators=[InputRequired()])
    mid_up_mid = HiddenField('mid_up_mid',validators=[InputRequired()])
    mid_up_right = HiddenField('mid_up_right',validators=[InputRequired()])
    mid_left = HiddenField('mid_left',validators=[InputRequired()])
    mid_mid = HiddenField('mid_mid',validators=[InputRequired()])
    mid_right = HiddenField('mid_right',validators=[InputRequired()])
    down_left = HiddenField('down_left',validators=[InputRequired()])
    down_mid = HiddenField('down_mid',validators=[InputRequired()])
    down_right = HiddenField('down_right',validators=[InputRequired()])


class FormSelectCoords(Form):
    x1 = HiddenField('x1', validators=[InputRequired()])
    x2 = HiddenField('x2', validators=[InputRequired()])
    y1 = HiddenField('y1', validators=[InputRequired()])
    y2 = HiddenField('y2', validators=[InputRequired()])
    
class Tegn4Form(Form):
    name = HiddenField('name', validators=[InputRequired()])
    coords = FieldList(FormField(FormSelectCoords))