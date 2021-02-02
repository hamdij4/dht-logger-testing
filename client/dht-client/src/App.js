import React from 'react';
import { makeStyles } from '@material-ui/core/styles';
import Paper from '@material-ui/core/Paper';
import Grid from '@material-ui/core/Grid';
import {useState, useEffect} from 'react';
import Card from '@material-ui/core/Card';
import CardActions from '@material-ui/core/CardActions';
import CardContent from '@material-ui/core/CardContent';
import Button from '@material-ui/core/Button';
import Typography from '@material-ui/core/Typography';
import * as Axios from 'axios';
import { HomeIcon, GitHub } from '@material-ui/icons';
const useStyles = makeStyles((theme) => ({
  root: {
    flexGrow: 1,
    minWidth: 275,
  },
  paper: {
    padding: theme.spacing(2),
    textAlign: 'center',
    color: theme.palette.text.secondary,
  },
  bullet: {
    display: 'inline-block',
    margin: '0 2px',
    transform: 'scale(0.8)',
  },
  title: {
    fontSize: 14,
  },
  pos: {
    marginBottom: 12,
  },
}));

export default function CenteredGrid() {
  const classes = useStyles();

  const [temp, setTemp] = useState("");
  const [humidity, setHumidity] = useState("");
  const [uptime, setUptime] = useState("");
  const [res, setRes] = useState("");
  const [refresh, setRefresh] = useState(false);

  useEffect(() => {  
    fetchData();
}, [refresh])
const bull = <span className={classes.bullet}>•</span>;
  const fetchData = async() => {
    await Axios.get('/client/data', {})
    .then(res => {
        setTemp(res.data.Temperature);
        setHumidity(res.data.Humidity);
        setUptime(res.data.Uptime);
        setRes(JSON.stringify(res.data));
    })
    .catch(error => {
        console.log(error)
    })
    .finally( () => {
    })
  }

  return (
    <div className={classes.root}>
      <Grid container spacing={3} style={{'margin-top': '75px'}}>
        <Grid item xs={3}>
        </Grid>
        <Grid item xs={6}>
          <Card className={classes.root} variant="outlined">
          <CardContent>
          <Typography className={classes.title} color="textSecondary" gutterBottom>
            Project submission
          </Typography>
          <Typography variant="h5" component="h2">
            DHT{bull}Logger{bull}Live
          </Typography>
          <Typography className={classes.pos} color="textSecondary">
            By Hamdija Radoncic for RTOS 2021
          </Typography>
          <Typography variant="body2" component="p">
            Uptime : { uptime ? uptime : 0 }
            <br />
            Latest request data : { res ? res : "none yet" }
            <br />
          <GitHub color="primary" style={{'cursor' : 'pointer'}} onClick={()=>{window.open("https://github.com/hamdij4/dht-logger-testing", "_blank")}}/>
          </Typography>
        </CardContent>
        <CardActions style={{'justifyContent' : 'center'}}>
          <Button size="small"  onClick={()=>{setRefresh(!refresh)}}>Refresh</Button>
          <Button size="small"  onClick={()=>{window.location.href="http://192.168.220.128/sd-card"}}>View SD Card</Button>
        </CardActions>
      </Card>
        </Grid>
        <Grid item xs={3}>
        </Grid>
        <Grid item xs={3}>
        </Grid>
        <Grid item xs={3}>
          <Paper className={classes.paper}>Temp : { temp ? temp : 0 }</Paper>
        </Grid>
        <Grid item xs={3}>
          <Paper className={classes.paper}>Humidity : { humidity ? humidity : 0 }</Paper>
        </Grid>
        <Grid item xs={3}>
        </Grid>
      </Grid>
    </div>
  );
}