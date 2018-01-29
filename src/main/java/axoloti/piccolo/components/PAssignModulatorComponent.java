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
package axoloti.piccolo.components;

import axoloti.piccolo.PUtils;
import axoloti.piccolo.patch.PatchPNode;
import axoloti.piccolo.patch.object.parameter.PParameterInstanceViewFrac32UMap;
import axoloti.preferences.Theme;
import axoloti.utils.Constants;
import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.Point;
import javax.swing.JPopupMenu;
import org.piccolo2d.event.PBasicInputEventHandler;
import org.piccolo2d.event.PInputEvent;
import org.piccolo2d.util.PPaintContext;

/**
 *
 * @author Johannes Taelman
 */
public class PAssignModulatorComponent extends PatchPNode {

    private static final Dimension dim = new Dimension(16, 12);

    final PParameterInstanceViewFrac32UMap parameterInstanceView;

    public PAssignModulatorComponent(PParameterInstanceViewFrac32UMap parameterInstanceView) {
        super(parameterInstanceView.getPatchView());
        setMinimumSize(dim);
        setMaximumSize(dim);
        setPreferredSize(dim);
        setSize(dim);
        this.parameterInstanceView = parameterInstanceView;
        addInputEventListener(new PBasicInputEventHandler() {
            @Override
            public void mouseClicked(PInputEvent e) {
                if (getVisible()) {
                    doPopup(e);
                    e.setHandled(true);
                }
            }
        });
    }

    void doPopup(PInputEvent e) {
        JPopupMenu sub2 = new JPopupMenu();
        PAssignModulatorMenuItems mi = new PAssignModulatorMenuItems(parameterInstanceView, sub2);
        Point popupLocation = PUtils.getPopupLocation(e);
        sub2.show(parameterInstanceView.getCanvas(), popupLocation.x, popupLocation.y);
    }

    @Override
    protected void paint(PPaintContext paintContext) {
        Graphics2D g2 = paintContext.getGraphics();
        g2.setFont(Constants.FONT);
        g2.setColor(Theme.getCurrentTheme().Object_Default_Background);
        g2.fillRect(1, 1, (int) getWidth(), (int) getHeight());
        if (parameterInstanceView.getModel().getModulators() != null) {
            g2.setColor(Theme.getCurrentTheme().Component_Primary);
            g2.fillRect(1, 1, 8, (int) getHeight());
            g2.setColor(Theme.getCurrentTheme().Component_Secondary);
        } else {
            g2.setColor(Theme.getCurrentTheme().Component_Primary);
        }
        PUtils.setRenderQualityToHigh(g2);
        g2.drawString("M", 1, (int) getHeight() - 2);
        PUtils.setRenderQualityToLow(g2);
        g2.setColor(Theme.getCurrentTheme().Component_Primary);
        final int rmargin = 2;
        final int htick = 2;
        int[] xp = new int[]{(int) getWidth() - rmargin - htick * 2, (int) getWidth() - rmargin, (int) getWidth() - rmargin - htick};
        final int vmargin = 4;
        int[] yp = new int[]{vmargin, vmargin, vmargin + htick * 2};
        g2.fillPolygon(xp, yp, 3);
    }
}
