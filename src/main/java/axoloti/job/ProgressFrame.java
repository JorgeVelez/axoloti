package axoloti.job;

/**
 *
 * @author jtaelman
 */
public class ProgressFrame extends javax.swing.JFrame implements IProgressReporter {

    /**
     * Creates new form ProgressDialog
     */
    public ProgressFrame() {
        super();
        initComponents();
        jProgressBar.setMinimum(0);
        jProgressBar.setMaximum(16384);
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jProgressBar = new javax.swing.JProgressBar();
        jLabelMsg = new javax.swing.JLabel();

        getContentPane().setLayout(new javax.swing.BoxLayout(getContentPane(), javax.swing.BoxLayout.PAGE_AXIS));

        jProgressBar.setAlignmentX(0.0F);
        getContentPane().add(jProgressBar);

        jLabelMsg.setText("jLabel1");
        jLabelMsg.setMaximumSize(new java.awt.Dimension(4500, 16));
        jLabelMsg.setPreferredSize(new java.awt.Dimension(4500, 16));
        getContentPane().add(jLabelMsg);

        pack();
    }// </editor-fold>//GEN-END:initComponents

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel jLabelMsg;
    private javax.swing.JProgressBar jProgressBar;
    // End of variables declaration//GEN-END:variables

    @Override
    public void setNote(String note) {
        jProgressBar.setString(note);
        setVisible(true);
    }

    @Override
    public void setProgress(float progress) {
        //System.out.println(String.format("progress      %3.3f", progress));
        jProgressBar.setValue((int) (progress * 16384));
    }

    @Override
    public void setProgressIndeterminate() {
        jProgressBar.setIndeterminate(true);
    }

    @Override
    public void setReady() {
        throw new UnsupportedOperationException("Not supported yet.");
    }

}