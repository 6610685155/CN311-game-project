##กรณีใช้ hotspot แล้วไม่ได้

ทดสอบให้รู้ชัดว่าใช่ firewall ไหม
ที่เครื่อง Server ปิด Windows Firewall ชั่วคราวเพื่อทดสอบ — เปิด cmd แบบ Run as administrator (คลิกขวาที่ cmd เลือก Run as administrator) แล้วพิมพ์:

netsh advfirewall set allprofiles state off

แล้วลองเล่นใหม่ ถ้าคราวนี้ต่อติด = ยืนยันว่า firewall คือตัวปัญหา
ถ้าใช่ firewall ให้เปิดกลับแล้วเพิ่ม rule แทน
อย่าปล่อยปิดทิ้งไว้ ให้เปิดกลับ แล้วเพิ่มกฎอนุญาตเฉพาะพอร์ต 5000 (ใน cmd admin เครื่อง Server):

netsh advfirewall set allprofiles state on
netsh advfirewall firewall add rule name="Battleship" dir=in action=allow protocol=TCP localport=5000