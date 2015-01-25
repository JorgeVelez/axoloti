/**
 * Copyright (C) 2013, 2014 Johannes Taelman
 *
 * This file is part of Axoloti.
 *
 * Axoloti is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Axoloti is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Axoloti. If not, see <http://www.gnu.org/licenses/>.
 */
package axoloti.attribute;

import axoloti.attributedefinition.AxoAttribute;
import axoloti.attributedefinition.AxoAttributeComboBox;
import axoloti.object.AxoObjectInstance;
import axoloti.utils.Constants;
import components.DropDownComponent;
import org.simpleframework.xml.Attribute;

/**
 *
 * @author Johannes Taelman
 */
public class AttributeInstanceComboBox extends AttributeInstanceString {

    @Attribute(name = "selection", required = false)
    String selection;
    DropDownComponent comboBox;

    public AttributeInstanceComboBox() {
    }

    public AttributeInstanceComboBox(AxoAttribute param, AxoObjectInstance axoObj1) {
        super(param, axoObj1);
    }

    @Override
    public void PostConstructor() {
        super.PostConstructor();
//        final DefaultComboBoxModel model = new DefaultComboBoxModel(((AxoAttributeComboBox) attr).getMenuEntries().toArray());
        comboBox = new DropDownComponent(((AxoAttributeComboBox) attr).getMenuEntries());
        comboBox.setFont(Constants.font);
        setString(selection);
        comboBox.addItemListener(new DropDownComponent.DDCListener() {
            @Override
            public void SelectionChanged() {
                selection = (String) comboBox.getSelectedItem();
            }
        });
        this.add(comboBox);
    }

    @Override
    public void Lock() {
        if (comboBox != null) {
            comboBox.setEnabled(false);
        }
    }

    @Override
    public void UnLock() {
        if (comboBox != null) {
            comboBox.setEnabled(true);
        }
    }

    @Override
    public String CValue() {
        String s = ((AxoAttributeComboBox) attr).getCEntries().get(comboBox.getSelectedIndex());
        if (s != null) {
            return s;
        } else {
            return "";
        }
    }

    @Override
    public String getString() {
        return selection;
    }

    @Override
    public void setString(String selection) {
        this.selection = selection;
        if (comboBox == null) {
            return;
        }
        if (selection == null) {
            this.selection = (String) comboBox.getItemAt(0);
        }
        comboBox.setSelectedItem(this.selection);
        if (this.selection.equals((String) comboBox.getSelectedItem())) {
            return;
        }
        for (int i = 0; i < comboBox.getItemCount(); i++) {
            if (this.selection.equals(comboBox.getItemAt(i))) {
                this.selection = comboBox.getItemAt(i);
                return;
            }
        }
    }
}