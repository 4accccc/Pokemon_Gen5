/*
 * POIを使用して xlsをxml形式に変換するコンバータ k.ohno 2009.03.10
 * 参考にしたファイル
 * http://fiehnlab.ucdavis.edu/staff/wohlgemuth/java/document.2005-09-01.0039862784
 */

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.apache.poi.hssf.usermodel.HSSFCell;
import org.apache.poi.hssf.usermodel.HSSFRow;
import org.apache.poi.hssf.usermodel.HSSFSheet;
import org.apache.poi.hssf.usermodel.HSSFWorkbook;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

/**
 *
 */
public class XLS2XML {
    XMLOutputter outputter;

    private boolean header = false;


  public String getHEADER(int rowno, int colno){
    return "cell";
//      return "cell row=\"" + rowno + "\" col=\"" + colno +  "\"";
  }

  
    /**
     * converts the inputfile to an xml file
     *
     * @throws Exception
     *
     * @see edu.ucdavis.genomics.metabolomics.binbase.meta.converter.AbstractConverter#convert(java.io.InputStream,
     *      java.io.OutputStream)
     */
    public void convert(InputStream in) throws Exception {
        HSSFWorkbook workbook = new HSSFWorkbook(in);

        Document document = new Document();
        Element root = new Element("excel_workbook");
        document.setRootElement(root);

        for (int a = 0; a < workbook.getNumberOfSheets(); a++) {
            HSSFSheet sheet = workbook.getSheetAt(a);

            int min = sheet.getFirstRowNum();
            int max = sheet.getLastRowNum();


          String[] header = null;

          Element sheetElement = new Element("sheet");
          Element elementp = new Element("page");
          Element elementpt = new Element("pagetitle");
          elementp.setText(String.valueOf(a));
          elementpt.setText(workbook.getSheetName(a));
          sheetElement.addContent(elementp);
          sheetElement.addContent(elementpt);
          
 //           sheetElement.setAttribute("name", workbook.getSheetName(a));
//            sheetElement.setAttribute("index", String.valueOf(a));

            for (int i = min; i < max; i++) {
                HSSFRow row = sheet.getRow(i);

                if (i == min) {

                    if (this.header == true) {
                        // header row
                        short firstCell = row.getFirstCellNum();
                        short lastCell = row.getLastCellNum();
                        header = new String[lastCell];

                        for (short x = firstCell; x < lastCell; x++) {
                            HSSFCell cell = row.getCell(x);


                            if(cell == null) {
                                header[x] = "NoHeader";
                            }
                            else {
                            switch (cell.getCellType()) {
                            case HSSFCell.CELL_TYPE_BLANK:
                                header[x] = "NoHeader";
                                break;
                            case HSSFCell.CELL_TYPE_STRING:
                                header[x] = cell.getStringCellValue();
                                break;

                            default:
                                throw new IOException("cell type is not as header supported");
                            }
                            }
                        }
                    } else {
                        // header row
                        short firstCell = row.getFirstCellNum();
                        short lastCell = row.getLastCellNum();
                        header = new String[lastCell];

                        for (short x = firstCell; x < lastCell; x++) {
                            header[x] = getHEADER(i,x);
                        }
                    }
                }
               
                if(this.header == false){
                    createElement(header, sheetElement, row);  
                }
                else if(i > min) {
                    createElement(header, sheetElement, row);
                }
               
            }
            root.addContent(sheetElement);
        }
        outputter.output(document, System.out);

    }

    /**
     * @param header
     * @param sheetElement
     * @param row
     */
  private void createElement(String[] header, Element sheetElement, HSSFRow row) {
    short firstCell = row.getFirstCellNum();
    short lastCell = row.getLastCellNum();
    
    Element element = new Element("row");
    
    for (short x = firstCell; x < lastCell; x++) {
      HSSFCell cell = row.getCell(x);

      if( header.length > x){
      
      if ( header[x].equals("NoHeader") == false) {
        Element content = new Element(header[x].replaceAll(" ", "_"));

        content.setAttribute("row", String.valueOf(row.getRowNum()));
        content.setAttribute("col", String.valueOf(x));

        
        if(cell == null) {
          content.setText("");
        }
        else {
          switch (cell.getCellType()) {
          case HSSFCell.CELL_TYPE_BLANK:
//            content.setText("");
            continue;
//            break;
          case HSSFCell.CELL_TYPE_BOOLEAN:
            content.setText(String.valueOf(cell.getBooleanCellValue()).trim());
            break;
          case HSSFCell.CELL_TYPE_ERROR:
            content.setText(String.valueOf(cell.getErrorCellValue()).trim());
            break;
          case HSSFCell.CELL_TYPE_FORMULA:
            content.setText(String.valueOf(cell.getCellFormula()).trim());
            break;
          case HSSFCell.CELL_TYPE_NUMERIC:
            content.setText(String.valueOf(cell.getNumericCellValue()).trim());
            break;
          case HSSFCell.CELL_TYPE_STRING:
            content.setText(String.valueOf(cell.getStringCellValue()).trim());
            break;
          default:
          }
          element.addContent(content);
        }
//        element.addContent(content);
      }
      }
    }
    if(element.getContentSize() > 0){
      sheetElement.addContent(element);
    }
  }

    /**
     *
     */
    public XLS2XML() {
        super();


      Format f = Format.getPrettyFormat();
      f.setTextMode(Format.TextMode.NORMALIZE);
      f.setOmitDeclaration(true);
      f.setEncoding("Shift_JIS");
      outputter = new XMLOutputter(f);

      
   //     outputter.setNewlines(true);
     //   outputter.setIndent(true);
    }
   
    public static void main(String[] args) throws FileNotFoundException, Exception {
        new XLS2XML().convert(new FileInputStream(args[0]));
    }

    public boolean isHeader() {
        return header;
    }

    public void setHeader(boolean header) {
        this.header = header;
    }
}
