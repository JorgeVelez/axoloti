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
package axoloti.parameters;

import axoloti.datatypes.Int32;
import axoloti.datatypes.ValueInt32;
import java.security.MessageDigest;
import org.simpleframework.xml.Element;

/**
 *
 * @author Johannes Taelman
 */
public class ParameterInt32HRadio extends Parameter<Int32> {

    @Element
    ValueInt32 MaxValue;

    public ParameterInt32HRadio() {
    }

    public ParameterInt32HRadio(String name, int MinValue, int MaxValue) {
        super(name);
        this.MaxValue = new ValueInt32(MaxValue);
    }

    @Override
    public ParameterInstance InstanceFactory() {
        ParameterInstanceInt32HRadio b = new ParameterInstanceInt32HRadio();
        return b;
    }

    @Override
    public void updateSHA(MessageDigest md) {
        super.updateSHA(md);
        md.update(("int32.hradio" + MaxValue.getInt()).getBytes());
    }
}
