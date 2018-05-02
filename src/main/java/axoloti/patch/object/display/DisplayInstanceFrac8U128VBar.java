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
package axoloti.patch.object.display;

import axoloti.object.atom.AtomDefinitionController;
import axoloti.object.display.DisplayFrac8U128VBar;

/**
 *
 * @author Johannes Taelman
 */
public class DisplayInstanceFrac8U128VBar extends DisplayInstance<DisplayFrac8U128VBar> {

    final int n = 128;

    DisplayInstanceFrac8U128VBar(AtomDefinitionController controller) {
        super(controller);
    }

    public int[] getIDst() {
        return value;
    }

    public int getN() {
        return n;
    }

    private int[] value;

    @Override
    public Object getValue() {
        return value;
    }

    @Override
    public void setValue(Object value) {
        this.value = (int[]) value;
        firePropertyChange(DISP_VALUE, null, value);
    }

}
