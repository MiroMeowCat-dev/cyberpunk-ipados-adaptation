// Gyro controls inside the existing Zelda-derived settings, not a new overlay.
- (NSString *)tableView:(UITableView *)table titleForHeaderInSection:(NSInteger)section {
    return section==1?@"iPad 体感视角":@"虚拟手柄";
}
- (void)gyroCalibrationFinished:(NSNotification *)notification {
    [self.tableView reloadRowsAtIndexPaths:@[[NSIndexPath indexPathForRow:3 inSection:1]] withRowAnimation:UITableViewRowAnimationNone];
}
- (void)toggleGyro:(UISwitch *)toggle {
    [NSUserDefaults.standardUserDefaults setBool:toggle.on forKey:@"PortGyroEnabled"];
    if(!toggle.on)PortGyroSuspend();[self.tableView reloadData];
}
- (void)gyroMode:(UISegmentedControl *)control {
    [NSUserDefaults.standardUserDefaults setInteger:control.selectedSegmentIndex forKey:@"PortGyroMode"];
}
- (void)gyroGain:(UISlider *)slider {
    [NSUserDefaults.standardUserDefaults setDouble:slider.value forKey:@"PortGyroGain"];
    UITableViewCell *cell=[self.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:2 inSection:1]];
    cell.textLabel.text=[NSString stringWithFormat:@"灵敏度 %.2f×",slider.value];
}
- (void)gyroAxisGain:(UISlider *)slider {
    BOOL vertical=slider.tag==7;
    [NSUserDefaults.standardUserDefaults setDouble:slider.value forKey:vertical?@"PortGyroVerticalGain":@"PortGyroHorizontalGain"];
    UITableViewCell *cell=[self.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:slider.tag inSection:1]];
    cell.textLabel.text=[NSString stringWithFormat:vertical?@"纵向幅度 %.2f×":@"横向幅度 %.2f×",slider.value];
}
- (void)gyroInvert:(UISwitch *)toggle {
    [NSUserDefaults.standardUserDefaults setBool:toggle.on forKey:toggle.tag==4?@"PortGyroInvertX":@"PortGyroInvertY"];
}
- (UITableViewCell *)gyroCell:(UITableView *)table path:(NSIndexPath *)path {
    UITableViewCell *cell=[table dequeueReusableCellWithIdentifier:@"gyro"]?:[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:@"gyro"];
    cell.accessoryView=nil;cell.accessoryType=UITableViewCellAccessoryNone;
    cell.textLabel.textColor=UIColor.labelColor;cell.detailTextLabel.text=nil;
    cell.selectionStyle=UITableViewCellSelectionStyleNone;cell.userInteractionEnabled=YES;
    if(path.row==0) {
        cell.textLabel.text=@"启用体感";UISwitch *toggle=[UISwitch new];toggle.on=PortGyroEnabled();
        [toggle addTarget:self action:@selector(toggleGyro:) forControlEvents:UIControlEventValueChanged];cell.accessoryView=toggle;
    } else if(path.row==1) {
        cell.textLabel.text=@"启用方式";UISegmentedControl *mode=[[UISegmentedControl alloc] initWithItems:@[@"常开",@"仅瞄准"]];
        mode.selectedSegmentIndex=PortGyroMode();[mode addTarget:self action:@selector(gyroMode:) forControlEvents:UIControlEventValueChanged];cell.accessoryView=mode;
    } else if(path.row==2) {
        cell.textLabel.text=[NSString stringWithFormat:@"灵敏度 %.2f×",PortGyroGain()];UISlider *slider=[[UISlider alloc] initWithFrame:CGRectMake(0,0,210,32)];
        slider.minimumValue=.25;slider.maximumValue=4;slider.value=PortGyroGain();
        [slider addTarget:self action:@selector(gyroGain:) forControlEvents:UIControlEventValueChanged];cell.accessoryView=slider;
    } else if(path.row==3) {
        cell.textLabel.text=@"重新校准";cell.detailTextLabel.text=PortGyroStatus();cell.detailTextLabel.numberOfLines=2;
        cell.selectionStyle=UITableViewCellSelectionStyleDefault;cell.userInteractionEnabled=PortGyroEnabled()&&!gPhysicalMode;
        if(!cell.userInteractionEnabled)cell.textLabel.textColor=UIColor.tertiaryLabelColor;
    } else if(path.row>=6) {
        BOOL vertical=path.row==7;cell.textLabel.text=[NSString stringWithFormat:vertical?@"纵向幅度 %.2f×":@"横向幅度 %.2f×",PortGyroAxisGain(vertical)];
        UISlider *slider=[[UISlider alloc] initWithFrame:CGRectMake(0,0,210,32)];slider.minimumValue=.5;slider.maximumValue=2.5;
        slider.value=PortGyroAxisGain(vertical);slider.tag=path.row;
        [slider addTarget:self action:@selector(gyroAxisGain:) forControlEvents:UIControlEventValueChanged];cell.accessoryView=slider;
    } else {
        BOOL horizontal=path.row==4;cell.textLabel.text=horizontal?@"反转左右":@"反转上下";
        UISwitch *toggle=[UISwitch new];toggle.tag=path.row;toggle.on=[NSUserDefaults.standardUserDefaults boolForKey:horizontal?@"PortGyroInvertX":@"PortGyroInvertY"];
        [toggle addTarget:self action:@selector(gyroInvert:) forControlEvents:UIControlEventValueChanged];cell.accessoryView=toggle;
    }return cell;
}
